<template>
  <p>
    <span class="pr-2" v-if="cnd.status==1">⚠️</span>
    <span class="pr-2" v-if="cnd.status==2">✅</span>
    <span class="pr-2" v-if="cnd.status==3">❌</span>
    <b>{{ cnd.name }}</b>:
    <span :class="statusClass" v-if="cnd.status==1">
    {{props.t('safety.Conditions.home.inError')}} 
    </span>
    <span v-else >
      {{props.t('safety.Conditions.home.actualValue')}} 
      <span v-if="cnd.status == 2">
        {{props.t('safety.Conditions.home.is')}} 
      </span> 
      <span :class="statusClass" v-else>
        {{props.t('safety.Conditions.home.isnt')}} 
      </span>
      <span v-if="cnd.checkType == 0">{{props.t('safety.Conditions.home.minus')}}</span> 
      <span v-if="cnd.checkType == 1">{{props.t('safety.Conditions.home.minEqual')}}</span> 
      <span v-if="cnd.checkType == 2">{{props.t('safety.Conditions.home.equal')}}</span> 
      <span v-if="cnd.checkType == 3">{{props.t('safety.Conditions.home.greEqual')}}</span> 
      <span v-if="cnd.checkType == 4">{{props.t('safety.Conditions.home.greater')}}</span> 
      {{ cnd.refValue }}
    </span>
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
    case 2:
      return 'txt-green';
    default:
      return 'txt-red';
  }
});
</script>
