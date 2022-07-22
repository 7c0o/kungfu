import { createI18n } from 'vue-i18n';

import zh_CN from './zh-CN';
import en_US from './en-US';
import zh_HK from './zh-HK';

// 语言库
const messages = {
  'zh-CN': zh_CN,
  'en-US': en_US,
  'zh-HK': zh_HK,
};

export const languageList = [
  { value: 'zh-CN', label: '简体中文' },
  { value: 'en-US', label: 'English' },
];

// 默认语言
export const langDefault = process.env.LANG_ENV ?? 'zh-CN';

const i18n = createI18n({
  locale: langDefault, //默认显示的语言
  messages,
});

export default i18n;
