(ns agv-server.cljs.core
  (:require [cljs.core.async :refer [<! >! put! close! chan timeout]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(defn connect!
  "Connect the websocket, return a map of in and out data."
  [uri]
  (let [ws (js/WebSocket. uri)
        in (chan)
        out (chan)]
    (set! (.-onopen ws) (fn []
                          (js/console.log "opening socket")
                          (go-loop []
                            (let [data (<! in)]
                              (if (nil? data)
                                (do (close! out)
                                    (.close ws))
                                (do (.send ws (pr-str data))
                                    (recur)))))))
    (set! (.-onclose ws) (fn []
                           (js/console.log "closing socket")
                           (close! in)
                           (close! out)))
    (set! (.-onmessage ws) (fn [msg]
                             (when-let [data (.-data msg)]
                               (put! out data))))
    {:in in :out out}))


(let [{:keys [in out]} (connect! "ws://localhost:3000/ws")]
  (go-loop [] ;; GET DATA
    (let [data (<! out)]
      (if-not (nil? data)
        (do
          (js/console.log data)
          (recur)))))
  (go-loop [] ;; SEND DATA
   ; (<! (timeout 5000))
   ; (>! in "hejsan")
   ; (recur)))
    ))
