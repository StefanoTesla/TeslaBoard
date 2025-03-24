<template>
<div class="container" v-if="dataLoaded">
  <div class="container_header" @click="toggleOpen">
    <div class="title"><p>{{ moduleName}}</p></div>
    <div class="status">
      <div :class="['led',statusClass]"></div>
    </div>
    <div :class="['icon', open ? 'open' : '']">
      <svg fill="currentcolor" width="40px" height="40px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
        <g><g data-name="arrowhead-down"><rect width="24" height="24" opacity="0"/>
          <path d="M17.37 12.39L12 16.71l-5.36-4.48a1 1 0 1 0-1.28 1.54l6 5a1 1 0 0 0 1.27 0l6-4.83a1 1 0 0 0 .15-1.41 1 1 0 0 0-1.41-.14z"/>
          <path d="M11.36 11.77a1 1 0 0 0 1.27 0l6-4.83a1 1 0 0 0 .15-1.41 1 1 0 0 0-1.41-.15L12 9.71 6.64 5.23a1 1 0 0 0-1.28 1.54z"/>
          </g></g>
        </svg>
    </div>
  </div>
  <div class="body" v-if="open">
    <slot />
  </div>

</div>

<!-- Loading...-->
<div class="container" v-else>
  <div class="container_header">
      <div class="title"><p>Fetching Data...</p></div>
      <div class="status">
        <div :class="['led','red']"></div>
      </div>
      <div :class="['icon']">
        <svg fill="currentcolor" width="40px" height="40px" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
          <g data-name="Layer 2"><g data-name="arrowhead-down"><rect width="24" height="24" opacity="0"/>
            <path d="M17.37 12.39L12 16.71l-5.36-4.48a1 1 0 1 0-1.28 1.54l6 5a1 1 0 0 0 1.27 0l6-4.83a1 1 0 0 0 .15-1.41 1 1 0 0 0-1.41-.14z"/>
            <path d="M11.36 11.77a1 1 0 0 0 1.27 0l6-4.83a1 1 0 0 0 .15-1.41 1 1 0 0 0-1.41-.15L12 9.71 6.64 5.23a1 1 0 0 0-1.28 1.54z"/>
            </g></g>
          </svg>
      </div>
  </div>
</div>
</template>


<script setup>

import { ref } from 'vue';

defineProps({
  moduleName : String,
  dataLoaded : Boolean,
  statusClass: String
});


const open = ref(false);
  
  const toggleOpen = () => {
    open.value = !open.value
  }
  
</script>