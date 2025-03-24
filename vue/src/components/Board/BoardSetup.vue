<template>

    <Card
      v-if="props.txt.board"
      moduleName="TeslaBoard"
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

  const board = ref({})
  let dataLoaded = ref(false)
  let statusClass = ref('green')

</script>