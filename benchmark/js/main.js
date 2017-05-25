const fs = require('fs')
const csv = require('csv-parser')

let count = 0

fs.createReadStream(process.argv[2])
  .pipe(csv())
  .on('data', function(data) { ++count })
  .on('end', () => console.log(count))
