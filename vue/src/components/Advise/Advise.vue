<template>

  <ul>
    <li
      v-for="notify in notifies"
      :key="notify.id"
      :class="'notify ' + notify.type"
    >
      {{ notify.message }}
    </li>
  </ul>

</template>


<script setup>

const props = defineProps({
  t: Function,
  notifies: Object,
})


</script>