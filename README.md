college summoned me to make an app once again

# Weird runic hex matrix
*wrem for short*

note 1: if you're looking at this as an example for something, do not use Vue-drawing-canvas, it's unmaintained. You can use Konva instead, they even have an example, although it's more difficult to implement, you also get more freedom in terms of manipulating the canvas.<br>
note 2: shitty code, while I typically follow some practices, this is kinda crap because deadlines, it's not actually bad but beware before copying anything.<br>
note 3: using Wi-Fi instead of BLE here would make it require so much less optimizations and at least 1.2 times cooler.
note 4: android version works but is unfinished

![Megumin on a led matrix](https://github.com/user-attachments/assets/6090d2fa-2c0d-40d1-b58b-7fd0e92b79e8)


## What's this?
This a project that uses Vue, Tauri, Vue Drawing Canvas, Pako, Piko and a ESP32, Bluetooth Low Energy, Zlib + some extra stuff to display images on a 64x64 LED matrix irl.
Three modes:<br>
Manual - you draw on your canvas, press send to put your drawing on the matrix, with a buncha usual drawing stuff ofc.<br>
Real-time - it's more like 2 fps, but you get the gist, it's fun.<br>
Image - upload any image, including Megumin fan art, a lot of Megumin fan art actually.

## How?

Because no one invented a wi-fi library for Tauri yet and I have no idea if websockets would work properly on phone this project uses BLE instead. BLE isn't meant for transferring images like this so we have to split it into bits, ideally the smaller the better but I went for 32x32, which is 4 parts on my 64x64 matrix. There are a bunch of optimizations, we take the uint8 image array from the canvas, we resize that to the appropriate size with Piko so it looks good instead of the drawImage resize in canvas, then we yeet the alpha channel out of there, convert it to an RGB565 array, then use Pako to compress the hell of it and send it via BLE with Tauri-plugin-blec. Once the ESP32 device receives it, there's a lot of transformation magic, eventually we get a uint16 array that we can feed into the Adafruit Matrix library and finally draw our image. Skipped a lot of details but that's pretty much it. 

*Name note:
I'm aware hex starts with h and not e, you see it's an acronym with partial syllabic selection, not just an acronym.*<br>
*Icon note:
Don't think about it, and it won't haunt you (it's an inside joke you won't get it)*
