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

const sendCanvasImage = async () => {
  const canvasElem = <HTMLCanvasElement | null>document.getElementById("VueDrawingCanvas");
  if (!canvasElem) {return}

  const ctx = canvasElem.getContext('2d');
  if (!ctx) {return}

  // downscaling the image
  const dstCanvas = document.createElement('canvas');
  dstCanvas.width = 32;
  dstCanvas.height = 32;

  await info("yeeeeeee");
  await pica().resize(canvasElem, dstCanvas, {
    quality: 3,
    alpha: true,
  });
  await info("yeeeeeee");

  const dstCtx = dstCanvas.getContext('2d');
  if (!dstCtx) return;

  // converting image to a uint8Arr
  const imageData = dstCtx.getImageData(0, 0, 64, 64);

  const uint8Arr = new Uint8Array(imageData.data.buffer);


  await info("haaaaaaaaaaapchoo");
  await info(`ImageData length: ${uint8Arr.length}`);

  await info(`Ye: ${Array.from(uint8Arr.slice(0, 8)).join(', ')}`);
  await info("haaaaaaaaaaapchoo")

  await send(CHARACTERISTIC_UUID, uint8Arr);
}

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
      <button @click.prevent="async () => {await sendString(CHARACTERISTIC_UUID, 'abracadabra');}">Shut</button>
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
