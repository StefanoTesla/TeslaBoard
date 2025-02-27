import './style.css'
import Alpine from 'alpinejs'
import GlobalData from './globalData'
import BoardSetting from './Boardsetting'
import { Validator } from './utils/Validator'

window.Validator = Validator
window.Alpine = Alpine
window.GlobalData = GlobalData;
window.BoardSetting = BoardSetting;
Alpine.start()

