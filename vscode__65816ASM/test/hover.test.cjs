'use strict';
const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { parseDefinitions, resolveValue, getHover, getCompletions } = require('../hover');
const reference = parseDefinitions(fs.readFileSync(path.join(__dirname, '../data/registers.inc'), 'utf8'));
const hover = (text, word, defs = reference) => getHover(text, text.lastIndexOf(word) + Math.max(0, Math.floor(word.length / 2)), defs);

test('raw register address and canonical constant show the verified diagram', () => {
  for (const text of ['sta $2121', 'sta CGADD', 'start: STA.w CGADD']) {
    const result = hover(text, text.includes('$') ? '$2121' : 'CGADD');
    assert.equal(result.register, 'CGADD');
    assert.match(result.markdown, /AAAA AAAA/);
    assert.match(result.markdown, /SNESdev reference/);
    assert.match(result.markdown, /Memory write/);
  }
});

test('immediate numbers never inherit register documentation', () => {
  for (const text of ['lda #$2121', 'lda # $2121', 'lda #$2122', 'lda #$802121']) {
    const result = hover(text, text.match(/\$\w+/)[0]);
    assert.equal(result.register, undefined);
    assert.equal(result.immediate, true);
    assert.doesNotMatch(result.markdown, /SNESdev|palette|CGRAM/);
  }
});

test('immediate register symbols retain docs but explain there is no access', () => {
  const result = hover('lda #CGADD', 'CGADD');
  assert.equal(result.register, 'CGADD');
  assert.match(result.markdown, /does not access the register/);
  assert.match(result.markdown, /16-bit/);
});

test('custom .EQU aliases and alias chains inherit documentation by address', () => {
  for (const text of ['.EQU palettePort $2122\nsta palettePort', '.EQU palettePort CGDATA\n.EQU outputPort palettePort\nsta outputPort']) {
    const result = hover(text, text.split(' ').at(-1));
    assert.equal(result.register, 'CGDATA');
    assert.equal(result.value, 0x2122);
    assert.match(result.markdown, /low byte first/);
  }
});

test('comments, strings and comment-only definitions cannot create register hovers', () => {
  for (const text of ['; sta CGADD', 'sta CGADD ; $2121', '.DB "sta CGADD"', '/*\nsta CGADD\n*/', '/* .EQU fake $2121 */\nsta fake', '* sta CGADD', '// sta CGADD']) {
    const word = text.includes('fake') ? 'fake' : text.endsWith('$2121') ? '$2121' : 'CGADD';
    assert.equal(hover(text, word), undefined, text);
  }
});

test('raw pointer locations, indexed bases, calculations and branch targets are not direct accesses', () => {
  for (const text of ['lda ($2121)', 'lda [$2121]', 'sta $2121,x', 'sta $2121+1', 'jmp $2121', '.DW $2121', 'PEA $2121']) {
    assert.equal(hover(text, '$2121'), undefined, text);
  }
  assert.match(hover('lda (CGADD)', 'CGADD').markdown, /effective address depends/);
});

test('long addresses honor the SNES I/O bank mirrors', () => {
  assert.equal(hover('sta $802121', '$802121').register, 'CGADD');
  assert.equal(hover('sta $3F2121', '$3F2121').register, 'CGADD');
  assert.equal(hover('sta $7E2121', '$7E2121'), undefined);
  assert.equal(hover('sta $C02121', '$C02121'), undefined);
});

test('definitions, flag constants and symbol case are respected', () => {
  assert.equal(hover('.EQU CGADD $2121', 'CGADD').register, 'CGADD');
  assert.equal(hover('lda #BLANK_SCREEN', 'BLANK_SCREEN').register, undefined);
  assert.equal(hover('sta cgadd', 'cgadd'), undefined);
  assert.equal(hover('.equ CGADD $1234\nsta CGADD', 'CGADD').register, undefined);
  const cycle = parseDefinitions('.equ one two\n.equ two one');
  assert.equal(resolveValue('one', cycle), undefined);
});

test('workspace documentation can be edited without rebuilding the extension', () => {
  const workspace = new Map([...reference, ...parseDefinitions('; A new local explanation.\n.EQU CGADD $2121')]);
  assert.match(hover('sta CGADD', 'CGADD', workspace).markdown, /new local explanation/);
});

test('all hardware definitions have uppercase names, source links and bit diagrams', () => {
  let count = 0;
  for (const [name, item] of reference) {
    const value = resolveValue(item.expression, reference);
    if (value < 0x2100) continue;
    count++;
    assert.match(name, /^[A-Z][A-Z0-9]*$/);
    assert.match(item.documentation, /```text/);
    assert.match(item.documentation, /https:\/\/snes\.nesdev\.org\//);
  }
  assert.equal(count, 57);
  assert.equal(resolveValue('OBJSEL', reference), 0x2101);
  assert.equal(resolveValue('BG1SC', reference), 0x2107);
  assert.equal(resolveValue('APUIO0', reference), 0x2140);
});

test('completion offers canonical names and a PPU search category', () => {
  const items = getCompletions('sta CG', 6, reference);
  const cgadd = items.find(item => item.name === 'CGADD');
  assert.equal(cgadd.start, 4);
  assert.equal(cgadd.end, 6);
  assert.equal(cgadd.filterText, 'CGADD PPU');
  assert.match(cgadd.documentation, /AAAA AAAA/);
  assert.ok(items.some(item => item.name === 'CGDATA'));
  assert.ok(!items.some(item => item.name === 'CG_Addr'));
  assert.deepEqual(getCompletions('; CG', 4, reference), []);
  assert.deepEqual(getCompletions('.DB "CG', 7, reference), []);
  assert.deepEqual(getCompletions('; CG ', 5, reference), []);
  assert.deepEqual(getCompletions('/*\n  ', 5, reference), []);
  assert.deepEqual(getCompletions('sta $C', 6, reference), []);
  assert.equal(getCompletions('sta CGADD', 6, reference).find(item => item.name === 'CGADD').end, 9);
});
