const fs = require('fs')
const csv = require('csv-parser')

fs.createReadStream(process.argv[2])
  .pipe(csv())
  .on('data', function(data) { return }) // w/o this data is never piped and program just ends
  .on('end', () => console.log('done!'))
