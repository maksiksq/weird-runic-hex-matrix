<script setup lang="ts">
import { ref } from "vue";

const modeTxt = ref('Manual')

</script>

<template>
  <main>
    <section class="text-head">
      <h1>Weird runic hex matrix</h1>
    </section>
    <section class="button-seg">
      <button>{{ modeTxt }}</button>
      <button>Clear</button>
      <button>Shut</button>
    </section>
    <section>
      <section class="canvas-seg">
        <canvas class="canvas"></canvas>
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
    flex-direction: row;
    justify-content: center;
  }
}
</style>
