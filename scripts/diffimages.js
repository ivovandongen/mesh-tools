#!/usr/bin/env node
const fs = require('fs');
const PNG = require('pngjs').PNG;
const JPG = require('jpeg-js');
const pixelmatch = require('pixelmatch');

const args = process.argv.slice(2);
if (args.length !== 2) {
    console.log("Pass in 2 images to compare");
    process.exit(1);
}

function readImage(file) {
    const data = fs.readFileSync(file);
    if (file.endsWith(".png")) {
        return PNG.sync.read(data);
    } else if (file.endsWith(".jpg") || file.endsWith(".jpeg")) {
        return JPG.decode(data);
    } else {
        throw "Cannot read file: " + file;
    }
}

const img1 = readImage(args[0]);
const img2 = readImage(args[1]);
const {width, height} = img1;
const diff = new PNG({width, height});

const numDiffPixels = pixelmatch(img1.data, img2.data, diff.data, width, height, {threshold: 0.1});

fs.writeFileSync('diff.png', PNG.sync.write(diff));
console.log("Difference pixels:", numDiffPixels, " percentage:", (numDiffPixels / (width * height)) * 100);
