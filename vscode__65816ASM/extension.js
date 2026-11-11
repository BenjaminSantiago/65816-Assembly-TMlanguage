'use strict';

const vscode = require('vscode');
const fs = require('node:fs');
const path = require('node:path');
const { parseDefinitions, getHover, getCompletions } = require('./hover');

function activate(context) {
  const bundled = parseDefinitions(fs.readFileSync(path.join(context.extensionPath, 'data', 'registers.inc'), 'utf8'), 'bundled registers.inc');
  const files = new Map();
  const generations = new Map();
  const isRegisters = uri => /(?:^|\/)registers\.inc$/i.test(uri.path);
  const output = vscode.window.createOutputChannel('65816 Assembly');
  context.subscriptions.push(output);

  async function load(uri) {
    const key = uri.toString();
    const generation = (generations.get(key) || 0) + 1;
    generations.set(key, generation);
    try {
      const open = vscode.workspace.textDocuments.find(doc => doc.uri.toString() === key);
      const text = open ? open.getText() : Buffer.from(await vscode.workspace.fs.readFile(uri)).toString('utf8');
      if (generations.get(key) === generation) files.set(key, { uri, definitions: parseDefinitions(text, key) });
    } catch (error) {
      if (generations.get(key) === generation) files.delete(key);
      output.appendLine('Could not read ' + uri.fsPath + ': ' + error.message);
    }
  }

  let ready = Promise.resolve();
  function definitionsFor(document) {
    const folder = vscode.workspace.getWorkspaceFolder(document.uri);
    const definitions = new Map(bundled);
    for (const file of files.values()) {
      if (folder && vscode.workspace.getWorkspaceFolder(file.uri)?.uri.toString() === folder.uri.toString()) {
        for (const [name, item] of file.definitions) definitions.set(name, item);
      }
    }
    return definitions;
  }

  function markdown(text) {
    const value = new vscode.MarkdownString(text);
    value.isTrusted = false;
    value.supportHtml = false;
    return value;
  }

  function refresh() {
    ready = vscode.workspace.findFiles('**/registers.inc', '**/{.git,node_modules}/**').then(async uris => {
      const live = new Set(uris.map(uri => uri.toString()));
      for (const key of files.keys()) if (!live.has(key)) files.delete(key);
      await Promise.all(uris.map(load));
    }).catch(error => output.appendLine('Could not discover registers.inc: ' + error.message));
    return ready;
  }

  const watcher = vscode.workspace.createFileSystemWatcher('**/registers.inc');
  context.subscriptions.push(watcher,
    watcher.onDidCreate(load), watcher.onDidChange(load),
    watcher.onDidDelete(uri => {
      const key = uri.toString();
      generations.set(key, (generations.get(key) || 0) + 1);
      files.delete(key);
    }),
    vscode.workspace.onDidChangeTextDocument(event => { if (isRegisters(event.document.uri)) load(event.document.uri); }),
    vscode.workspace.onDidCloseTextDocument(document => { if (isRegisters(document.uri)) load(document.uri); }),
    vscode.workspace.onDidChangeWorkspaceFolders(refresh),
    vscode.languages.registerHoverProvider({ language: 'w65816' }, {
      async provideHover(document, position, token) {
        await ready;
        if (token.isCancellationRequested) return undefined;
        const result = getHover(document.getText(), document.offsetAt(position), definitionsFor(document));
        if (!result) return undefined;
        return new vscode.Hover(markdown(result.markdown), new vscode.Range(document.positionAt(result.start), document.positionAt(result.end)));
      }
    }),
    vscode.languages.registerCompletionItemProvider({ language: 'w65816' }, {
      async provideCompletionItems(document, position, token) {
        await ready;
        if (token.isCancellationRequested) return undefined;
        return getCompletions(document.getText(), document.offsetAt(position), definitionsFor(document)).map(candidate => {
          const item = new vscode.CompletionItem(candidate.name, vscode.CompletionItemKind.Constant);
          item.detail = candidate.detail;
          item.documentation = markdown(candidate.documentation);
          item.filterText = candidate.filterText;
          item.insertText = candidate.name;
          item.range = new vscode.Range(document.positionAt(candidate.start), document.positionAt(candidate.end));
          return item;
        });
      }
    })
  );
  refresh();
}

module.exports = { activate };
