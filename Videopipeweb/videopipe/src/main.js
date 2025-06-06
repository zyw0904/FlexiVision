import './assets/main.css'
import '@/assets/font/font.css'; 

import { createApp } from 'vue'
import App from './App.vue'
import ElementPlus from 'element-plus';
import 'element-plus/dist/index.css'
import router from './router';
import * as ElementPlusIconsVue from '@element-plus/icons-vue'
import drag from "v-drag"

const app = createApp(App)
for (const [key, component] of Object.entries(ElementPlusIconsVue)) {
    app.component(key, component)
  }

app.use(router);
app.use(ElementPlus);
app.use(drag);
app.mount('#app');

