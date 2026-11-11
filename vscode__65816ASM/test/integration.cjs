'use strict';
// Run using VS Code's --extensionDevelopmentPath and --extensionTestsPath.
const assert = require('node:assert/strict');
const vscode = require('vscode');

async function run() {
  const extension = vscode.extensions.getExtension('local.w65816-asm');
  assert.ok(extension, 'The 65816 development extension must be installed.');
  await extension.activate();
  const text = 'sta CGADD\nsta $2121\nlda #$2121\nlda #CGADD\nsta CGDATA\nsta INIDISP\n; CGADD\n';
  const document = await vscode.workspace.openTextDocument({ language: 'w65816', content: text });
  await vscode.window.showTextDocument(document);
  async function hover(line, character) {
    const items = await vscode.commands.executeCommand('vscode.executeHoverProvider', document.uri, new vscode.Position(line, character));
    return items.flatMap(item => item.contents.map(content => content.value || String(content))).join('\n');
  }
  assert.match(await hover(0, 5), /palette entry/);
  assert.match(await hover(1, 6), /AAAA AAAA/);
  const immediate = await hover(2, 7);
  assert.match(immediate, /Immediate/);
  assert.doesNotMatch(immediate, /SNESdev|CGRAM/);
  assert.match(await hover(3, 6), /does not access the register/);
  assert.match(await hover(4, 5), /low byte first/);
  assert.match(await hover(5, 5), /Forced|forced/);
  assert.equal(await hover(6, 4), '');

  const completionDoc = await vscode.workspace.openTextDocument({ language: 'w65816', content: 'sta CG\nsta PPU' });
  for (const line of [0, 1]) {
    const suggestions = await vscode.commands.executeCommand('vscode.executeCompletionItemProvider', completionDoc.uri, new vscode.Position(line, completionDoc.lineAt(line).text.length));
    const item = suggestions.items.find(item => item.label === 'CGADD');
    assert.ok(item, 'Register completion is registered.');
    assert.equal(item.insertText, 'CGADD');
    assert.equal(item.filterText, 'CGADD PPU');
    assert.match(item.documentation.value, /AAAA AAAA/);
  }

  const folder = vscode.workspace.workspaceFolders[0].uri;
  const registers = await vscode.workspace.openTextDocument(vscode.Uri.joinPath(folder, 'registers.inc'));
  const edit = new vscode.WorkspaceEdit();
  edit.insert(registers.uri, new vscode.Position(0, 0), '; Live workspace register documentation.\n.EQU LOCAL_PALETTE_PORT $2122\n\n');
  assert.equal(await vscode.workspace.applyEdit(edit), true);
  const local = await vscode.workspace.openTextDocument({ language: 'w65816', content: '.EQU OUTPUT_PORT CGDATA\nsta OUTPUT_PORT' });
  const items = await vscode.commands.executeCommand('vscode.executeHoverProvider', local.uri, new vscode.Position(1, 5));
  assert.match(items.flatMap(item => item.contents.map(content => content.value)).join('\n'), /low byte first/);

  const source = await vscode.workspace.openTextDocument(vscode.Uri.joinPath(folder, 'sample.asm'));
  const live = await vscode.commands.executeCommand('vscode.executeHoverProvider', source.uri, new vscode.Position(0, 6));
  assert.match(live.flatMap(item => item.contents.map(content => content.value)).join('\n'), /Live workspace register documentation/);
  console.log('PASS: real VS Code hovers and completion, immediate values, comments, aliases, and unsaved workspace documentation.');
}

module.exports = { run };
