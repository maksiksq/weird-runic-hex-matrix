<script setup lang="ts">
import {onMounted, ref, watch} from "vue";
import VueDrawingCanvas from "vue-drawing-canvas";
import {
  BleDevice,
  connect,
  disconnect,
  getConnectionUpdates,
  getScanningUpdates, send,
  sendString,
  startScan
} from "@mnlphlp/plugin-blec";
import {info} from "@tauri-apps/plugin-log";
import pica from "pica"

import { deflateRaw } from 'pako';

const modeTxt = ref('Manual')

const image = ref("");

const vueCanvas = ref<VueDrawingCanvas>(null);

// бомбокляд

const resizeCanvas = (): void => {
  // rip my reactivity
  const canvasElem = <HTMLCanvasElement | null>document.getElementById("VueDrawingCanvas");
  if (!canvasElem) {return}

  const img = new Image();
  img.src = image.value;


  canvasElem.width = window.innerWidth * 0.30;
  canvasElem.height = window.innerWidth * 0.30;

  const ctx = canvasElem.getContext('2d');


  ctx?.clearRect(0, 0, canvasElem.width, canvasElem.height);
  ctx?.drawImage(img, 0, 0);
}

const manuallyResetCanvas = (): void => {
  vueCanvas.value?.reset();

  const canvasElem = <HTMLCanvasElement | null>document.getElementById("VueDrawingCanvas");
  if (!canvasElem) {return}

  const ctx = canvasElem.getContext('2d');

  if (!ctx) {return}

  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, canvasElem.width, canvasElem.height);
  ctx.clearRect(0, 0, canvasElem.width, canvasElem.height);
}

const color = ref<string>()
const eraser = ref<boolean>(false);

onMounted(async () => {
  resizeCanvas();
  window.addEventListener("resize", resizeCanvas);

  await getConnectionUpdates((state) => connected.value = state)
  await getScanningUpdates((state) => {
    scanning.value = state
  })

  setInterval(async () => {
    await sendCanvasImage();
  }, 1000)
})

//

const connected = ref<Boolean>(false)
const scanning = ref<Boolean>(false)
const statusMsg = ref<string>("disconnected")

const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'

const devices = ref<BleDevice[]>([])

async function disconnectFromLock() {
  // disconnecting past connection in case it was connected
  if (connected.value) {
    await disconnect()
  }
}

const initiateBLE = async (): Promise<void> => {
  await disconnectFromLock();

  await startScan((dv: BleDevice[]) => {devices.value = dv;}, 10000);
}

watch(devices, async () => {
  console.log("haaaaaaaaaaapchoo");

  if (devices.value.length === 0) {
    statusMsg.value = "scan resulted in nothing";
  }

  const j = ref(0);
  for (const device of devices.value) {
    if (device.name === "ESP32_LED_Control") {
      await connect(device.address, () => console.warn('disconnected'));
      statusMsg.value = "connected";
    }
    if (j.value === devices.value.length - 1) {
      // nothing happens;
    }
    j.value += 1;
  }

  await sendCanvasImage()
})

const hashCanvasData = async (data: Uint8Array): Promise<string> => {
  for (let i = 0; i < data.length; i++) {
    data[i] = Math.round(data[i] / 4) * 4; // quantize to 4-bit resolution
  }
  const digest = await crypto.subtle.digest('SHA-1', data);
  return Array.from(new Uint8Array(digest)).map(b => b.toString(16).padStart(2, '0')).join('');
};

const stripAlpha = (data: Uint8Array): Uint8Array => {
  const rgb = new Uint8Array((data.length / 4) * 3);
  for (let i = 0, j = 0; i < data.length; i += 4, j += 3) {
    rgb[j] = data[i];     // R
    rgb[j + 1] = data[i + 1]; // G
    rgb[j + 2] = data[i + 2]; // B
    // notice the lack of alpha here
  }
  return rgb;
}

const rgb8ToRgb565 = (rgb8: Uint8Array): Uint8Array => {
  const length = rgb8.length / 3;
  const rgb565 = new Uint8Array(length * 2);

  for (let i = 0; i < length; i++) {
    const r = rgb8[i * 3];
    const g = rgb8[i * 3 + 1];
    const b = rgb8[i * 3 + 2];

    const rgb =
        ((r & 0b11111000) << 8) |
        ((g & 0b11111100) << 3) |
        (b >> 3);

    rgb565[i * 2] = (rgb >> 8) & 0xFF;     // High byte
    rgb565[i * 2 + 1] = rgb & 0xFF;        // Low byte
  }

  return rgb565;
}

const pastHashes = new Map<string, string>(); // key = `${row},${col}`

const sendCanvasImage = async () => {
  const canvasElem = document.getElementById("VueDrawingCanvas") as HTMLCanvasElement | null;
  if (!canvasElem) return;

  const ctx = canvasElem.getContext('2d');
  if (!ctx) return;

  const resizedCanvas = document.createElement('canvas');
  resizedCanvas.width = 64;
  resizedCanvas.height = 64;

  await pica().resize(canvasElem, resizedCanvas, {
    quality: 3,
    alpha: true,
  });

  const resizedCtx = resizedCanvas.getContext('2d');
  if (!resizedCtx) return;

  const vanillaResizedCanvas = document.createElement('canvas');
  vanillaResizedCanvas.width = 64;
  vanillaResizedCanvas.height = 64;
  const vanillaResizedCtx = vanillaResizedCanvas.getContext('2d');

  vanillaResizedCtx?.drawImage(canvasElem, 0, 0, 64, 64);

  const tileSize = 32;

  let allTilesUnchanged = true;

  for (let row = 0; row < 2; row++) {
    for (let col = 0; col < 2; col++) {
      const x = col * tileSize;
      const y = row * tileSize;

      const tileData = resizedCtx.getImageData(x, y, tileSize, tileSize);
      const vanillaResizedTileData = vanillaResizedCtx?.getImageData(x, y, tileSize, tileSize);

      if (!vanillaResizedTileData) continue;

      const resizedUint8Arr = new Uint8Array(tileData.data);
      const vanillaResizedUint8Arr = new Uint8Array(vanillaResizedTileData.data);
      const currentHash = await hashCanvasData(vanillaResizedUint8Arr);

      const key = `${row},${col}`;
      const previousHash = pastHashes.get(key);

      if (currentHash === previousHash) {
        await info(`Tile (${row}, ${col}) unchanged`);
        continue;
      }

      allTilesUnchanged = false;
      pastHashes.set(key, currentHash);

      // why not just disable alpha in pico? I still need to compare the vanilla canvas hashes with alpha unless I remake some logic
      const optimizedUint8Arr = rgb8ToRgb565(stripAlpha(resizedUint8Arr));
      const uint8Arr = new Uint8Array(optimizedUint8Arr.length + 2);
      // await info("alpha arr:");
      // await info(resizedUint8Arr.toString());
      // await info("alphaless arr:");
      // await info(optimizedUint8Arr.toString());
      // await info("Arr len:");
      // await info(uint8Arr.length.toString());

      uint8Arr.set(optimizedUint8Arr);
      uint8Arr[optimizedUint8Arr.length] = row;
      uint8Arr[optimizedUint8Arr.length + 1] = col;

      await info("sending this arr:");
      await info(uint8Arr.toString());

      await info(`Sending changed tile (${x}, ${y}), length: ${uint8Arr.length}`);
      await send(CHARACTERISTIC_UUID, uint8Arr);
    }
  }

  if (allTilesUnchanged) {
    await info("All tiles unchanged, no data sent.");
  } else {
    await info("All changed tiles sent.");
  }
};

watch(connected, async () => {
  if (connected.value) {
    statusMsg.value = "connected";
  } else {
    statusMsg.value = "disconnected";
  }
})

//

const onNewImg = async (e: Event): Promise<void> => {
  const file = e.target?.files[0]
  if (!file) return

  const canvasElem = <HTMLCanvasElement | null>document.getElementById("VueDrawingCanvas");
  if (!canvasElem) {return}

  const reader = new FileReader()
  reader.onload = () => {
    const img = new Image()
    img.onload = () => {
      const canvas = canvasElem
      const ctx = canvas.getContext('2d')
      canvas.width = img.width
      canvas.height = img.height
      ctx?.drawImage(img, 0, 0)
    }
    img.src = reader.result
  }
  reader.readAsDataURL(file)

};
</script>

<template>
  <main>
    <section class="text-head">
      <h1>Weird runic hex matrix</h1>
    </section>
    <section class="status-line">
      <p>Status: {{ statusMsg }}</p>
    </section>
    <section class="button-seg">
      <button>{{ modeTxt }}</button>
      <button @click.prevent="manuallyResetCanvas">Clear</button>
      <button @click.prevent="initiateBLE">Connect</button>
      <button @click.prevent="disconnectFromLock">Shut</button>
      <button @click.prevent="info('hapchooo')"></button>
      <button @click.prevent="sendCanvasImage">send</button>
    </section>
    <section>
      <section class="canvas-seg">
        <div class="toolbar">
          <input type="color" v-model="color">
          <select v-model="eraser">
            <option :value=false>Brush</option>
            <option :value=true>Eraser</option>
          </select>
          <input type="file" @change="onNewImg" accept="/image"/>
        </div>
        <div ref="canvasContainer" class="canvas-cont">
          <vue-drawing-canvas
              ref="vueCanvas"
              v-model:image="image"
              :line="4"
              :color="color"
              :eraser="eraser"
              :strokeType="line"
              :line-cap="square"
              :line-join="miter"
              saveAs="png"/>
        </div>
      </section>
    </section>
  </main>
</template>
<style>
* {
  font-family: Comfortaa, sans-serif;
  color: #fff;

  margin: 0;
  padding: 0;
}

body, html {
  overflow: hidden;
  background-color: rgba(29, 29, 33, 1);
}

::selection {
  background: oklch(0.6956 0.2047 20.75);
}

button, a {
  all: unset;
}
</style>
<style scoped>
@import url('https://fonts.googleapis.com/css2?family=Comfortaa:wght@300..700&display=swap');

main {
  height: 100%;

  section {
    display: flex;
    flex-direction: row;
    justify-content: center;
  }

  & .text-head {
    padding-top: 5%;

    & h1 {
      font-size: 1rem;
    }
  }

  & .status-line {
    padding-top: 1%;

    & p {
      font-size: 1rem;
    }
  }

  & .button-seg {
    padding-top: 3%;

    width: 100%;

    gap: 3vw;

    & button {
      cursor: pointer;
      padding: 0.6vw 1vw;

      font-size: 1rem;
      background-color: oklch(0.6956 0.2047 20.75);
    }
  }

  & .canvas-seg {
    display: flex;
    flex-direction: column;
    align-items: center;

    padding-top: 3%;

    width: 100%;

    & div {
      display: flex;
      flex-direction: row;
      justify-content: flex-start;

      width: 30vw;

      & input, select {
        border-radius: 0;
        border: solid black 1px;
      }

      & select {
        color: black;


        & option {
          color: black;
        }
      }
    }

    & .canvas-cont {
      border: solid black 2px;

      background-color: white;
      width: 30vw;
      height: 30vw;

      & canvas {
        width: 100%;
        height: 100%;
      }
    }
  }
}
</style>
