<template>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.pin }}</p>
    <div class="input_with_unit">
      <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" min=1 max=39 @change="validate()"/>
    </div>
  </div>
</template>


<script setup>

  import { ref,onMounted } from 'vue'
  import { toast } from 'vue3-toastify';
  import { useValidator } from '../../composables/Validator';

  const { isInvalidPin, isGreaterThan, isNegative } = useValidator();

 const props = defineProps({
   txt: Object,
   index: Number,
   swi: Object
 })
 const emit = defineEmits(['update:validated','update:pinUsed']);

 let pinUnvalid = ref(false)

 const validate = () => {
  emit('update:validated', { index: props.index, isValid: false});
  pinUnvalid.value = false
  if (isInvalidPin(props.swi.pin,'output')){
      pinUnvalid.value = true
      const errorMessage = props.txt.gen.pin + " " + props.swi.pin + " - " +  props.txt.errors.gpio.noUsableAsOutput
      errorResponseNotify(errorMessage)
      return
  }

  emit('update:validated', { index: props.index, isValid: true});
  emit('update:pinUsed', { pin: props.swi.pin, type:3, index:props.index});
 }

 const errorResponseNotify = (errorMessage) => {
   toast.error(errorMessage, {
   autoClose: 3000,
 });
}

 onMounted(()=>{
  props.swi.pin = props.swi.pin ?? null
  validate()
 })

</script>