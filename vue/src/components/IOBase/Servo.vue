<template>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.pin }}</p>
    <div class="input_with_unit">
      <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" min=1 max=39 @change="validate()"/>
    </div>
  </div>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.Servo.movingTime }}</p>
    <div class="input_with_unit">
      <span class="unit">sec</span><input :id="`sw_${index}_movTime`" :class="['with_unit', movTimeUnvalid ? 'validation_error' : '']" type="number" v-model="swi.moveTime" @change="validate()"/>
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
   swi: {
    type: Object,
    default: () => ({
        moveTime: 0
      })
    }
 })
 const emit = defineEmits(['update:validated','update:pinUsed']);

 let pinUnvalid = ref(false)
 let movTimeUnvalid = ref(false)


 const validate = () => {
  emit('update:validated', { index: props.index, isValid: false});
  props.swi.pin = parseInt(props.swi.pin)
  pinUnvalid.value = false
   if (isInvalidPin(props.swi.pin,'input')){
       pinUnvalid.value = true
       const errorMessage = props.txt.gen.pin + " " + props.swi.pin + " - " +  props.txt.errors.gpio.noUsableAsOtput
       errorResponseNotify(errorMessage)
       return
   }


   props.swi.moveTime = parseInt(props.swi.moveTime)
   movTimeUnvalid.value = false
   if (isNegative(props.swi.moveTime)){
    movTimeUnvalid.value = true 
    errorResponseNotify(props.txt.errors.general.negativeValue)
    return
   }

   emit('update:validated', { index: props.index, isValid: true});
   emit('update:pinUsed', { pin: props.swi.pin, type:4, index:props.index});
 }

 const errorResponseNotify = (errorMessage) => {
   toast.error(errorMessage, {
   autoClose: 3000,
 });
}

 onMounted(()=>{
  props.swi.pin = props.swi.pin ?? null
  props.swi.moveTime = props.swi.moveTime ?? 0;
  validate()
 })

</script>