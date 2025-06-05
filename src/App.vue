<script setup lang="ts">
import {onMounted, ref, watch} from "vue";
import VueDrawingCanvas from "vue-drawing-canvas";
import {BleDevice, connect, sendString, startScan} from '@mnlphlp/plugin-blec'

const devices = ref<BleDevice[]>([])

watch(devices, async () => {

  if (devices.value.length === 0) {
      status.value = "scan resulted in nothing";
  }

  const j = ref(0);
  for (const device of devices.value) {
    if (device.name === "ESP32_LED_Control") {
      await connect(device.address, () => console.warn('disconnected'));
      status.value = "connected";
    }
    if (j.value === devices.value.length - 1) {
      status.value = 'no matrix in the vicinity';
    }
    j.value += 1;
  }

  await sendString(CHARACTERISTIC_UUID, "on");
})

const modeTxt = ref('Manual')

const image = ref("");

const vueCanvas = ref<VueDrawingCanvas>(null);

// бомбокляд

const resizeCanvas = (): void => {
  console.log(vueCanvas.value);
  console.log("aaa");
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

onMounted(() => {
  resizeCanvas();
  window.addEventListener("resize", resizeCanvas);
})

//

const status = ref<string>("disconnected")

const CHARACTERISTIC_UUID = '51FF12BB-3ED8-46E5-B4F9-D64E2FEC021B'
await sendString(CHARACTERISTIC_UUID, 'on', 'withResponse')

</script>

<template>
  <main>
    <section class="text-head">
      <h1>Weird runic hex matrix</h1>
    </section>
    <section class="status-line">
      <p>Status: {{ status }}</p>
    </section>
    <section class="button-seg">
      <button>{{ modeTxt }}</button>
      <button @click.prevent="manuallyResetCanvas">Clear</button>
      <button @click.prevent="await startScan((dv: BleDevice[]) => {devices.value = dv;}, 10000);">Connect</button>
      <button @click.prevent="">Shut</button>
    </section>
    <section>
      <section class="canvas-seg">
        <div class="toolbar">
          <input type="color" v-model="color">
          <select v-model="eraser">
            <option :value=false>Brush</option>
            <option :value=true>Eraser</option>
          </select>
        </div>
        <div ref="canvasContainer" class="canvas-cont">
          <vue-drawing-canvas
              ref="vueCanvas"
              v-model:image="image"
              :line="4"
              :color="color"
              :eraser="eraser"
              :strokeType="'line'"
              :line-cap="'square'"
              :line-join="'miter'"
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
