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
  txt: Object,
  notifies: Object,
})


</script>