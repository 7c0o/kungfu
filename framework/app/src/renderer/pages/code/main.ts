import './setEnv';
import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/code/App.vue';
import store from '../index/store';
import { Input, Button, Layout, Dropdown, Menu } from 'ant-design-vue';
import 'monaco-editor/min/vs/editor/editor.main.css';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { loadCustomFont } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';

const app = createApp(App);

app.component('ComFileNode');

app.use(Input).use(Button).use(Layout).use(Dropdown).use(Menu).use(store);

app.use(VueI18n);

loadCustomFont().then(() => app.mount('#app'));

window.fileId = 0;
