<template>

  <Card
    v-if="props.txt.dome"
    :moduleName="props.txt.dome.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >

  </Card>
</template>


<script setup>
import { ref } from 'vue'
import { toast } from 'vue3-toastify';
import Card from '../Card.vue';

const props = defineProps({
  txt: Object
})

const switches = ref({})
let dataLoaded = ref(false)
let statusClass = ref('green')

</script>