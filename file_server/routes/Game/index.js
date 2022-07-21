const express = require('express');
const router = express.Router();
const fs = require('fs');
const path = require('path');
var chokidar = require('chokidar');
const readFilePath = path.join(__dirname, "..", "..", "files", "data_r.txt");
const writeFilePath = path.join(__dirname, "..", "..", "files", "data_w.txt");
var watcher = chokidar.watch(writeFilePath, { ignored: /^\./, persistent: true });

const handleFileChange = (path) => {
    console.log('File', path, 'has been changed');
    try {
        const data = fs.readFileSync(writeFilePath, 'utf8');
        try {
            fs.writeFileSync(readFilePath, data);
            // file written successfully
        } catch (err) {
            console.error("Write error ", err);
        }
        res.json(data);
    } catch (err) {
        console.error("Read error ", err);
    }
}
watcher.on('change', handleFileChange);
// .on('add', function (path) { console.log('File', path, 'has been added'); })
// .on('unlink', function (path) { console.log('File', path, 'has been removed'); })
// .on('error', function (error) { console.error('Error happened', error); })

// Post one new
router.post('/move', (req, res) => {

    try {
        const data = fs.readFileSync(readFilePath, 'utf8');
        try {
            fs.writeFileSync(writeFilePath, data);
            // file written successfully
        } catch (err) {
            console.error("Write error ", err);
        }
        res.json(data);
    } catch (err) {
        console.error("Read error ", err);
    }
});
router.get('/', (req, res) => {
    console.log(req);
    res.send("This is /game endpoint!");
})
module.exports = router;