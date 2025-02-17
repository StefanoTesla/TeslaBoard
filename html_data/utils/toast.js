export function toast() {
    return {
        addToast(notice) {
              setTimeout(() => {
                notice.id = Date.now()
                this.notices.push(notice)
                this.fireToast(notice.id,notice.time)
            }, 1 );},
            
        fireToast(id,time = 0) {
            this.visible.push(this.notices.find(notice => notice.id == id))
            let timeShown = 500
            if (time != 0){
              timeShown = 1000 * time
            }
            
            setTimeout(() => {
              this.removeToast(id)
            }, timeShown)
          },
        removeToast(id) {
            const notice = this.visible.find(notice => notice.id == id)
            const index = this.visible.indexOf(notice)
            this.visible.splice(index, 1)
          },
    
    }
}