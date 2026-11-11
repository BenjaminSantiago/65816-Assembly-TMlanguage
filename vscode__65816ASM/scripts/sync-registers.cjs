'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { parseDefinitions } = require('../hover');
const source = process.argv[2];
if (!source) throw new Error('Usage: npm run sync-registers -- /path/to/src/__inc/registers.inc');
const text = fs.readFileSync(source, 'utf8');
const definitions = parseDefinitions(text);
for (const name of ['INIDISP', 'CGADD', 'CGDATA']) {
  if (!definitions.has(name)) throw new Error('Missing expected definition: ' + name);
}
fs.writeFileSync(path.join(__dirname, '..', 'data', 'registers.inc'), text);
console.log('Copied ' + definitions.size + ' definitions and their documentation into the extension.');
