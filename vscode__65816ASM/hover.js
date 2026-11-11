'use strict';

// Keep source offsets intact while excluding comments and strings from hovers.
function maskNonCode(text) {
  return text.replace(/\/\*[\s\S]*?(?:\*\/|(?![\s\S]))|\/\/[^\r\n]*|;[^\r\n]*|^\*[^\r\n]*|"(?:\\.|[^"\\\r\n])*(?:"|(?=\r?$))|'(?:\\.|[^'\\\r\n])*(?:'|(?=\r?$))/gm,
    match => match.replace(/[^\r\n]/g, ' '));
}

const DEFINITION = /^\s*\.(?:EQU|DEFINE)\s+([A-Za-z_][\w@]*)\s+(?:=\s*)?([^;\r\n]+?)\s*$/i;
const MEMORY_OPS = new Set('ADC AND ASL BIT CMP CPX CPY DEC EOR INC LDA LDX LDY LSR ORA ROL ROR SBC STA STX STY STZ TRB TSB'.split(' '));
const WRITE_OPS = new Set('STA STX STY STZ'.split(' '));
const MODIFY_OPS = new Set('ASL DEC INC LSR ROL ROR TRB TSB'.split(' '));

function parseDefinitions(text, source = '') {
  const raw = text.split(/\r?\n/);
  const code = maskNonCode(text).split(/\r?\n/);
  const result = new Map();
  let comments = [];
  raw.forEach((line, index) => {
    const comment = line.match(/^\s*;(?: ?)(.*)$/);
    if (comment && !code[index].trim()) {
      comments.push(comment[1]);
      return;
    }
    const match = code[index].match(DEFINITION);
    if (match) {
      const inline = line.indexOf(';');
      if (inline >= 0) comments.push(line.slice(inline + 1).trim());
      result.set(match[1], { name: match[1], expression: match[2].trim(), documentation: comments.join('\n').trim(), source, line: index });
    }
    comments = [];
  });
  return result;
}

function resolveValue(expression, definitions, seen = new Set()) {
  const value = expression.trim();
  if (/^\$[\da-f]+$/i.test(value)) return parseInt(value.slice(1), 16);
  if (/^0x[\da-f]+$/i.test(value)) return parseInt(value.slice(2), 16);
  if (/^%[01]+$/.test(value)) return parseInt(value.slice(1), 2);
  if (/^\d+$/.test(value)) return Number(value);
  if (seen.has(value) || !definitions.has(value)) return undefined;
  seen.add(value);
  return resolveValue(definitions.get(value).expression, definitions, seen);
}

function registerAddress(value) {
  if (!Number.isInteger(value) || value < 0 || value > 0xffffff) return undefined;
  const bank = value >>> 16;
  if (bank > 0x3f && (bank < 0x80 || bank > 0xbf)) return undefined;
  const address = value & 0xffff;
  if ((address >= 0x2100 && address <= 0x21ff) || (address >= 0x4000 && address <= 0x43ff)) return address;
  return undefined;
}

function hex(value) {
  return '$' + value.toString(16).toUpperCase().padStart(value > 0xffff ? 6 : value > 0xff ? 4 : 2, '0');
}

function getHover(text, offset, referenceDefinitions) {
  const code = maskNonCode(text);
  const lineStart = code.lastIndexOf('\n', offset - 1) + 1;
  const newline = code.indexOf('\n', offset);
  const line = code.slice(lineStart, newline < 0 ? code.length : newline);
  const column = offset - lineStart;
  const tokens = /\$[\da-f]+|0x[\da-f]+|%[01]+|\b\d+\b|[A-Za-z_][\w@]*/gi;
  let token;
  for (const candidate of line.matchAll(tokens)) {
    if (column >= candidate.index && column < candidate.index + candidate[0].length) { token = candidate; break; }
  }
  if (!token) return undefined;

  const definitions = new Map([...referenceDefinitions, ...parseDefinitions(text)]);
  const name = token[0];
  const definition = definitions.get(name);
  const value = resolveValue(name, definitions);
  if (value === undefined) return undefined;

  const declaration = line.match(DEFINITION);
  // A colon label is optional. Labels and opcodes themselves are not operands.
  const instruction = line.match(/^\s*(?:[A-Za-z_][\w@]*:\s*)?([A-Za-z]{3})(?:\.[bwl])?\s+(\S.*)$/i);
  const operand = instruction && instruction[2].trim();
  const mnemonic = instruction && instruction[1].toUpperCase();
  const operandStart = instruction ? line.lastIndexOf(instruction[2]) : -1;
  const immediate = operand && operand.startsWith('#');
  const direct = operand && /^(?:\$[\da-f]+|0x[\da-f]+|\d+|[A-Za-z_][\w@]*)(?:\.[bwl])?$/i.test(operand);
  const memory = instruction && MEMORY_OPS.has(mnemonic) && direct && token.index >= operandStart;

  if (instruction && token.index < operandStart) return undefined;
  if (!declaration && !instruction) return undefined;
  // A raw address inside an indexed/indirect/expression operand is not the effective address.
  if (!definition && !declaration && !immediate && !memory) return undefined;

  const address = registerAddress(value);
  // Prefer the hovered symbol's docs. Aliases inherit reference docs by address.
  const registers = [...referenceDefinitions.values()].filter(item =>
    address !== undefined && registerAddress(resolveValue(item.expression, referenceDefinitions)) === address);
  const register = registers.find(item => item.name === name) || registers.find(item => item.documentation.includes('SNESdev reference')) || registers[0];
  const showRegister = Boolean(register) && (Boolean(definition) || Boolean(declaration) || Boolean(memory)) && !(immediate && !definition);
  if (!definition && !showRegister && !immediate) return undefined;

  let markdown = '**' + (definition ? name : showRegister ? register.name : 'Immediate value') + '** — `' + hex(value) + '`';
  if (showRegister && register.name !== name && definition) markdown += ' · ' + register.name;
  if (immediate) {
    markdown += '\n\nImmediate operand (`#`): uses this value; it does not access the register.';
    if (!/^#\s*(?:\$[\da-f]+|0x[\da-f]+|\d+|[A-Za-z_][\w@]*)(?:\.[bwl])?$/i.test(operand)) {
      markdown += '\n\nThe value shown is for this symbol or literal before byte selection or arithmetic.';
    }
    if (value > 0xff && ['LDA', 'LDX', 'LDY'].includes(mnemonic)) markdown += '\n\nThis full value requires a 16-bit destination register.';
  } else if (memory) {
    markdown += '\n\n' + (WRITE_OPS.has(mnemonic) ? 'Memory write.' : MODIFY_OPS.has(mnemonic) ? 'Read/modify/write operation.' : 'Memory read.');
    if (value <= 0xffff && showRegister) markdown += ' Register mapping assumes a SNES I/O data bank.';
  } else if (declaration) {
    markdown += '\n\nSymbolic constant definition.';
  } else {
    markdown += '\n\nSymbol reference in an operand; the effective address depends on the addressing mode and expression.';
  }

  if (showRegister) {
    markdown += '\n\n' + register.documentation;
    if (definition && definition.documentation && definition.documentation !== register.documentation) {
      markdown += '\n\n**Local definition**\n\n' + definition.documentation;
    }
  } else if (definition && definition.documentation) {
    markdown += '\n\n' + definition.documentation;
  } else {
    markdown += '\n\nDecimal: `' + value + '` · Binary: `%' + value.toString(2) + '`';
  }
  return { markdown, start: lineStart + token.index, end: lineStart + token.index + name.length, value, register: showRegister ? register.name : undefined, immediate: Boolean(immediate) };
}

function getCompletions(text, offset, referenceDefinitions) {
  const code = maskNonCode(text);
  // A probe at the caret detects comments/strings even after trailing whitespace.
  if (!maskNonCode(text.slice(0, offset) + 'X').endsWith('X')) return [];
  const start = code.lastIndexOf('\n', offset - 1) + 1;
  const prefix = code.slice(start, offset);
  if (!prefix.trim() && text.slice(start, offset).trim()) return [];
  const partial = prefix.match(/[A-Za-z_][\w@]*$/)?.[0] || '';
  if (partial && /[$%]/.test(code[offset - partial.length - 1] || ' ')) return [];
  const end = offset + (partial ? code.slice(offset).match(/^[\w@]*/)[0].length : 0);
  const definitions = new Map([...referenceDefinitions, ...parseDefinitions(text)]);
  return [...definitions.values()].flatMap(item => {
    const value = resolveValue(item.expression, definitions);
    if (value === undefined) return [];
    const category = value >= 0x2100 && value <= 0x213f ? 'PPU' : value >= 0x4300 && value <= 0x437f ? 'DMA' : value >= 0x2140 && value <= 0x2143 ? 'APU' : value >= 0x4000 && value <= 0x43ff ? 'CPU' : '';
    return [{ name: item.name, detail: hex(value) + (category ? ' · ' + category + ' register' : ' · constant'), documentation: item.documentation,
      filterText: item.name + (category ? ' ' + category : ''), start: offset - partial.length, end }];
  });
}

module.exports = { maskNonCode, parseDefinitions, resolveValue, registerAddress, getHover, getCompletions };
