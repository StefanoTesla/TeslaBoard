import { createApp } from "vue";
import "./style.css";
import App from "./Home.vue";
import Vue3Toastify, { toast } from "vue3-toastify";
import "vue3-toastify/dist/index.css";

createApp(App)
  .use(Vue3Toastify, {
    autoClose: 1000,
    expandCustomProps: true,
    theme: "dark",
  })
  .mount("#app");
