<template>

    <p>{{ props.txt.IOBase.pin }}</p>
    <div class="input_with_unit">
      <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" min=1 max=39 @change="validate()"/>
    </div>
    <p>{{ props.txt.IOBase.Servo.maxDeg }}</p>
    <select :id="`sw_${index}_invert`" :class="[maxDegUnvalid ? 'validation_error' : '']" v-model="swi.maxDeg" @change="validate()">
       <option v-for="[key, value] in Object.entries(props.txt.IOBase.Servo.maxDegEnum)" :key="key" :value="key">
         {{ value }}
       </option>
     </select>
    <p>{{ props.txt.IOBase.Servo.openDeg }}</p>
    <div class="input_with_unit">
      <span class="unit">°</span><input :id="`sw_${index}_open`" :class="['with_unit', openDegUnvalid ? 'validation_error' : '']" type="number" v-model="swi.openDeg" @change="validate()"/>
    </div>
    <p>{{ props.txt.IOBase.Servo.closeDeg }}</p>
    <div class="input_with_unit">
      <span class="unit">°</span><input :id="`sw_${index}_close`" :class="['with_unit', closeDegUnvalid ? 'validation_error' : '']" type="number" v-model="swi.closeDeg" @change="validate()"/>
    </div>
    <p>{{ props.txt.IOBase.Servo.movingTime }}</p>
    <div class="input_with_unit">
      <span class="unit">sec</span><input :id="`sw_${index}_movTime`" :class="['with_unit', movTimeUnvalid ? 'validation_error' : '']" type="number" v-model="swi.movTime" @change="validate()"/>
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
        maxDeg: 90,
        openDeg: 0,
        closeDeg: 0,
        movTime: 0
      })
    }
 })
 const emit = defineEmits(['update:validated']);

 let pinUnvalid = ref(false)
 let openDegUnvalid = ref(false)
 let closeDegUnvalid = ref(false)
 let maxDegUnvalid = ref(false)
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

   props.swi.maxDeg = parseInt(props.swi.maxDeg)
   maxDegUnvalid.value = false
   if (isNegative(props.swi.maxDeg)){
    maxDegUnvalid.value = true
       errorResponseNotify(props.txt.errors.general.negativeValue)
       return
   }
   if (isGreaterThan(props.swi.maxDeg, 270)){
    maxDegUnvalid.value = true
       const errorMessage = props.txt.errors.general.greaterThan + " " + props.swi.maxDeg
       errorResponseNotify(errorMessage)
       return
   }

   openDegUnvalid.value = false
   props.swi.openDeg = parseInt(props.swi.openDeg)
   if (isNegative(props.swi.openDeg)){
    openDegUnvalid.value = true
       errorResponseNotify(props.txt.errors.general.negativeValue)
       return
   }
   
   if (isGreaterThan(props.swi.openDeg, props.swi.maxDeg)){
    openDegUnvalid.value = true
    const errorMessage = props.txt.errors.general.greaterThan + " " + props.swi.maxDeg
    errorResponseNotify(errorMessage)
    return
   }

   props.swi.closeDeg = parseInt(props.swi.closeDeg)
   closeDegUnvalid.value = false
   if (isNegative(props.swi.closeDeg)){
    closeDegUnvalid.value = true 
    errorResponseNotify(props.txt.errors.general.negativeValue)
    return
   }
   
   if (isGreaterThan(props.swi.closeDeg, props.swi.maxDeg)){
    closeDegUnvalid.value = true
    const errorMessage = props.txt.errors.general.greaterThan + " " + props.swi.maxDeg
    errorResponseNotify(errorMessage)
    return
   }

   props.swi.movTime = parseInt(props.swi.movTime)
   movTimeUnvalid.value = false
   if (isNegative(props.swi.movTime)){
    movTimeUnvalid.value = true 
    errorResponseNotify(props.txt.errors.general.negativeValue)
    return
   }

   emit('update:validated', { index: props.index, isValid: true});

 }

 const errorResponseNotify = (errorMessage) => {
   toast.error(errorMessage, {
   autoClose: 3000,
 });
}

 onMounted(()=>{
  props.swi.pin = props.swi.pin ?? null
  props.swi.maxDeg = props.swi.maxDeg ?? 90;
  props.swi.openDeg = props.swi.openDeg ?? 0;
  props.swi.closeDeg = props.swi.closeDeg ?? 0;
  props.swi.movTime = props.swi.movTime ?? 0;
  console.log("mount")
  validate()
 })

</script>