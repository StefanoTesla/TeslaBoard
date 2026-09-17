<template>
  <p :class="statusClass"><b>{{ cnd.name }}</b>: {{props.t('safety.Conditions.home.actualValue')}} 
    <span v-if="cnd.status == 2">{{props.t('safety.Conditions.home.is')}} </span> 
    <span v-if="cnd.status == 3">{{props.t('safety.Conditions.home.isnt')}} </span>

    <span v-if="cnd.checkType == 0">{{props.t('safety.Conditions.home.minus')}}</span> 
    <span v-if="cnd.checkType == 1">{{props.t('safety.Conditions.home.minEqual')}}</span> 
    <span v-if="cnd.checkType == 2">{{props.t('safety.Conditions.home.equal')}}</span> 
    <span v-if="cnd.checkType == 3">{{props.t('safety.Conditions.home.greEqual')}}</span> 
    <span v-if="cnd.checkType == 4">{{props.t('safety.Conditions.home.greater')}}</span>

     {{ cnd.refValue }}
  </p>
</template>

<script setup>
import { computed } from 'vue';

const props = defineProps({
  t: Function,
  cnd: Object,
});


const statusClass = computed(() => {

  switch (props.cnd?.status) {
    case 1:
      return 'cnd-error';
    case 2:
      return 'cnd-issafe';
    case 3:
      return 'cnd-unsafe';
    default:
      return '';
  }
});
</script>
