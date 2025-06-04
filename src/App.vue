<script setup lang="ts">
import {onMounted, ref} from "vue";
import VueDrawingCanvas from "vue-drawing-canvas";

const modeTxt = ref('Manual')

const image = ref("");

const VueCanvasDrawing = ref<VueDrawingCanvas>(null);

// бомбокляд

const resizeCanvas = (): void => {
  console.log("got here")

  const canvasElem = <HTMLCanvasElement | null>document.getElementById("VueDrawingCanvas");
  if (!canvasElem) {return}

  console.log(image.value);
  const img = new Image();
  img.src = image.value;
  console.log(img.src);


  canvasElem.width = window.innerWidth * 0.30;
  canvasElem.height = window.innerWidth * 0.30;

  const ctx = canvasElem.getContext('2d');

  ctx?.clearRect(0, 0, canvasElem.width, canvasElem.height);
  ctx?.drawImage(img, 0, 0);
}

const color = ref<string>()
const eraser = ref<boolean>(false);

onMounted(() => {
  resizeCanvas();
  window.addEventListener("resize", resizeCanvas);
})

</script>

<template>
  <main>
    <section class="text-head">
      <h1>Weird runic hex matrix</h1>
    </section>
    <section class="button-seg">
      <button>{{ modeTxt }}</button>
      <button @click="VueCanvasDrawing?.reset()">Clear</button>
      <button>Shut</button>
    </section>
    <section>
      <section class="canvas-seg">
        <div class="toolbar">
          <input type="color" v-model="color">
          <select @click="console.log(eraser); console.log('eeeeeeeee')" v-model="eraser">
            <option :value=false>Brush</option>
            <option :value=true>Eraser</option>
          </select>
        </div>
        <div ref="canvasContainer" class="canvas-cont">
          <vue-drawing-canvas
              ref="VueCanvasDrawing"
              v-model:image="image"
              :line="4"
              :width="550"
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

  & .text-head {
    padding-top: 6%;

    display: flex;
    flex-direction: row;
    justify-content: center;

    & h1 {
      font-size: 1rem;
    }
  }

  & .button-seg {
    padding-top: 6%;

    width: 100%;

    display: flex;
    flex-direction: row;
    justify-content: center;
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

    padding-top: 6%;

    width: 100%;

    & div {
      display: flex;
      flex-direction: row;
      justify-content: flex-start;

      & input {

      }
    }

    & .canvas-cont {
      background-color: white;
      width: 30vw;
      height: 30vw;
    }
  }
}
</style>
