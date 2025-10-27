<template>
  <Card
    v-if="props.txt.dome"
    :moduleName="props.txt.dome.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <p class="title">Configurazione Modulo</p>
    <div class="card mb-4">
      <div class="setting_table">

        <div class="flex">
          
          <div class="txt pr-4">
            {{ props.txt.gen?.moduleIs }}
          </div>
          <div class="module_toggle">
            <label class="toggle" for="dome_module_status">
              <input
                class="toggle__input"
                name=""
                type="checkbox"
                id="dome_module_status"
                v-model="dome.enable"
                @change="validate()"
              />
              <div class="toggle__fill"></div>
            </label>
          </div>
        </div>

        <div class="txt pr-4">
          Posizione nella homepage:
          <select id="board_locale" v-model="dome.uiOrder">
            <option :value="1">1</option>
            <option :value="2">2</option>
            <option :value="3">3</option>
          </select>
        </div>
        <div class="txt pr-4">
          Nome modulo:
            <input
                  :id="`dome_module_name`"
                  type="text"
                  v-model="dome.identifier"
                />
        </div>
      </div>
    </div>
    <div v-if="dome.enable">
      <p class="title">Shutter</p>
      <div class="grid grid-cols-1 gap-4 md:grid-cols-2">
        <!-- driver type -->
        <div class="card">
          <div class="setting_table">
            <div class="sw_header">
              <p>{{ props.txt.dome.setting.driverType }}</p>
            </div>
            <div class="setting_row">
              <select
                :id="`dome_cmd_type`"
                :class="[boardTypeUnvalid ? 'validation_error' : '']"
                v-model="dome.shutter.driverType"
                @change="validate()"
              >
                <option
                  v-for="[key, value] in Object.entries(
                    props.txt.dome.setting.driverTypeEnum
                  )"
                  :key="key"
                  :value="key"
                >
                  {{ value }}
                </option>
              </select>
            </div>
          </div>
        </div>

        <!-- timeout -->
        <div class="card">
          <div class="setting_table">
            <div class="sw_header">
              <p>{{ props.txt.dome.setting.timeOutMoviment }}</p>
            </div>
            <div class="setting_row">
              <div class="input_with_unit">
                <span class="unit">sec</span
                ><input
                  :id="`dome_timeout`"
                  :class="[
                    'with_unit',
                    movTimeOutUnvalid ? 'validation_error' : '',
                  ]"
                  type="number"
                  v-model="dome.shutter.travelTOut"
                  @change="validate()"
                />
              </div>
            </div>
          </div>
        </div>

        <!-- open input-->
        <div class="card">
          <p class="title">{{ props.txt.dome.setting.inOpen }}</p>
          <Input
            :txt="props.txt"
            :index="0"
            :swi="dome.shutter.pinOpen"
            @update:validated="handleValidation"
            @update:pinUsed="updatePin"
          />
        </div>

        <!-- close input -->
        <div class="card">
          <p class="title">{{ props.txt.dome.setting.inClose }}</p>

          <Input
            :txt="props.txt"
            :index="1"
            :swi="dome.shutter.pinClose"
            @update:validated="handleValidation"
            @update:pinUsed="updatePin"
          />
        </div>

        <!--out Start/Open/Movement -->
        <div class="card">
          <p class="title" v-if="dome.shutter.driverType == 0">
            {{ props.txt.dome.setting.outStart }}
          </p>
          <p class="title" v-if="dome.shutter.driverType == 1">
            {{ props.txt.dome.setting.outOpen }}
          </p>
          <p class="title" v-if="dome.shutter.driverType == 2">
            {{ props.txt.dome.setting.outMove }}
          </p>

          <Output
            :txt="props.txt"
            :index="2"
            :swi="dome.shutter.outStart"
            @update:validated="handleValidation"
            @update:pinUsed="updatePin"
          />
        </div>
        <!--out Halt/Close/Direction -->
        <div class="card">
          <p class="title" v-if="dome.shutter.driverType == 0">
            {{ props.txt.dome.setting.outHalt }}
          </p>
          <p class="title" v-if="dome.shutter.driverType == 1">
            {{ props.txt.dome.setting.outClose }}
          </p>
          <p class="title" v-if="dome.shutter.driverType == 2">
            {{ props.txt.dome.setting.outDir }}
          </p>

          <Output
            :txt="props.txt"
            :index="3"
            :swi="dome.shutter.outHalt"
            @update:validated="handleValidation"
            @update:pinUsed="updatePin"
          />
        </div>
      </div>
      <!-- auto close -->
      <div class="card my-4">
        <div>
          <div class="flex items-center justify-evenly">
            <p class="font-bold">{{ props.txt.dome.setting.autoClose }}</p>
            <label class="toggle" for="dome_autoclose_enable">
              <input
                class="toggle__input"
                name=""
                type="checkbox"
                id="dome_autoclose_enable"
                v-model="dome.shutter.autoclose.enable"
                @change="validate()"
              />
              <div class="toggle__fill"></div>
            </label>
          </div>
          <div v-if="dome.shutter.autoclose.enable">
            <p>{{ props.txt.dome.setting.autoclosewarning1 }}</p>
            <p>{{ props.txt.dome.setting.autoclosewarning2 }}</p>
            <p>{{ props.txt.dome.setting.autoclosewarning3 }}</p>
            <p>{{ props.txt.dome.setting.autoclosewarning4 }}</p>
            <p>{{ props.txt.dome.setting.autoclosewarning5 }}</p>
            <div class="setting_row">
              <div class="input_with_unit">
                <span class="unit">min</span
                ><input
                  :id="`dome_autoclose_minutes`"
                  :class="[
                    'with_unit',
                    autoCloseTimeUnvalid ? 'validation_error' : '',
                  ]"
                  type="number"
                  v-model="dome.shutter.autoclose.time"
                  @change="validate()"
                />
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">
        {{ props.txt.gen?.loadFromBoard }}
      </button>
      <button
        :class="[
          validationState ? 'red cursor-pointer' : 'black cursor-not-allowed',
        ]"
        @click="saveData()"
      >
        {{ props.txt.gen?.save }}
      </button>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, watch } from "vue";
import { toast } from "vue3-toastify";
import Card from "../Card.vue";
import Input from "../IOBase/Input.vue";
import Output from "../IOBase/Output.vue";
import { useValidator } from "../../composables/Validator";

const props = defineProps({
  txt: Object,
  reboot: Boolean,
  pinUsed: Array,
});

const { isGreaterThan, isNegative } = useValidator();

const emit = defineEmits(["update:reboot", "update:pinUsed"]);

const dome = ref({});

const originalData = ref({});
let dataLoaded = ref(false);
let statusClass = ref("green");
let validation = ref([]);
let pinUsed = ref([]);
let validationState = ref(true);
let boardTypeUnvalid = ref(false);
let movTimeOutUnvalid = ref(false);
let autoCloseTimeUnvalid = ref(false);

const handleValidation = (data) => {
  const { index, isValid } = data;
  validation.value[index] = isValid;
};

const getOriginal = () => {
  dome.value = JSON.parse(JSON.stringify(originalData.value));
};

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/dome/cfg");
    if (!response.ok) {
      throw new Error("Network response was not ok");
    }
    const data = await response.json();
    dome.value = data;
    dataLoaded.value = true;

    updatePinsforObserver(data);
    if (data.reboot) {
      statusClass.value = "orange";
    }

    originalData.value = JSON.parse(JSON.stringify(dome.value));
  } catch (error) {
    console.error("Errore durante la chiamata API:", error);
  }
};

const validate = () => {
  validationState.value = false;
  statusClass.value = "red";

  dome.value.position = parseInt(dome.value.position);
  if (!dome.value.enable) {
    statusClass.value = "green";
    validationState.value = true;
    return;
  }

  dome.value.shutter.driverType = parseInt(dome.value.shutter.driverType);
  boardTypeUnvalid.value = false;

  if (isNegative(dome.value.shutter.driverType)) {
    boardTypeUnvalid.value = true;
    errorResponseNotify(props.txt.errors.general.negativeValue);
    return;
  }

  
  if (isGreaterThan(dome.value.shutter.driverType, 2)) {
    boardTypeUnvalid.value = true;
    const errorMessage = props.txt.errors.general.greaterThan + " " + "2";
    errorResponseNotify(errorMessage);
    return;
  }



  dome.value.shutter.travelTOut = parseInt(dome.value.shutter.travelTOut);
  movTimeOutUnvalid.value = false;
  if (isNegative(dome.value.shutter.travelTOut)) {
    movTimeOutUnvalid.value = true;
    errorResponseNotify(props.txt.errors.general.negativeValue);
    return;
  }


    if(dome.value.shutter.autoclose.enable){

      dome.value.shutter.autoclose.time = parseInt(dome.value.shutter.autoclose.time);
      autoCloseTimeUnvalid.value = false;
      if (isNegative(dome.value.shutter.autoclose.time)) {
        autoCloseTimeUnvalid.value = true;
        errorResponseNotify(props.txt.errors.general.negativeValue);
        return;
      }

    }


  const containsFalse = validation.value.some((item) => item === false);
  if (containsFalse) {
    return;
  }

  statusClass.value = "green";
  validationState.value = true;
};

const saveData = async () => {
  if (!validationState.value && dome.value.enable) {
    errorResponseNotify(props.txt.errors.general.validationFailed);
    return;
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/dome/cfg", {
      method: "POST",
      headers: {
        Accept: "application/json, text/plain, */*",
        "Content-Type": "application/json",
      },
      body: JSON.stringify(dome.value),
    });

    const data = await response.json();
    if (!response.ok) throw { status: response.status, data };

    cmdExecutedNotify();

    if (data.reboot) {
      console.log("reboot needed");
      emit("update:reboot", true);
      statusClass.value = "orange";
    }
  } catch (err) {
    if (err?.status === 500 && Array.isArray(err.data?.errors)) {
      err.data.errors.forEach((e) =>
        typeof e === "object"
          ? handleStructuredError(e)
          : errorResponseNotify(e)
      );
    } else {
      errorResponseNotify(
        err?.message || props.txt.errors.general.configRejected
      );
    }
  }
};

const handleStructuredError = (e) => {
  let msg;
  let pinName;
  switch (e.id) {
    case 1:
      pinName = props.txt.dome.setting.inOpen;
      break;
    case 2:
      pinName = props.txt.dome.setting.inClose;
      break;
    case 3:
      pinName = props.txt.dome.setting.outStart;
      break;
    case 4:
      pinName = props.txt.dome.setting.outHalt;
      break;
    default:
      pinName = `Pin ${e.id}`;
  }

  msg = pinName + ": " + props.txt.errors.gpioValidation[e.error];
  errorResponseNotify(msg);
};

const cmdExecutedNotify = () => {
  toast.success(props.txt.gen.configSaved, {
    autoClose: 500,
  });
};

const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};

onMounted(() => {
  fetchData();
});

watch(
  () => validation.value.some((item) => item === false),
  (containsFalse) => {
    statusClass.value = "red";
    validationState.value = containsFalse ? false : true;
    if (validationState.value) {
      validate();
    }
  }
);

const updatePinsforObserver = (data) => {
  pinUsed.value = [];
  /*
  pinUsed.value[0]={"pin":data.shutter.pinOpen.pin,"type":1,"module": 2}
  pinUsed.value[1]={"pin":data.shutter.pinClose.pin,"type":1,"module": 2}
  pinUsed.value[2]={"pin":data.shutter.pinStart.pin,"type":2,"module": 2}
  pinUsed.value[3]={"pin":data.shutter.pinHalt.pin,"type":2,"module": 2}
  emit('update:pinUsed', pinUsed.value);
  */
};

const updatePin = (data) => {
  pinUsed.value[data.index] = { pin: data.pin, type: data.type, module: 1 };
  emit("update:pinUsed", pinUsed.value);
};
</script>
