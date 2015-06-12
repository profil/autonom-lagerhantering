(ns agv-server.cljs.websocket
  (:require [cljs.core.async :refer [<! put! close! chan]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))


(defn connect!
  "Connect the websocket, return a map of in and out data."
  [uri]
  (let [ws (js/WebSocket. uri)
        ready (chan)
        in (chan)
        out (chan)]
    (set! (.-onopen ws)
      (fn []
        (close! ready)
        (js/console.log "opening socket to uri:" uri)
        (go-loop []
          (let [data (<! in)]
            (if (nil? data)
              (do (close! out)
                  (.close ws))
              (do (.send ws (pr-str data))
                  (recur)))))))
    (set! (.-onclose ws)
      (fn []
        (js/console.log "closing socket to uri:" uri)
        (close! in)
        (close! out)))
    (set! (.-onmessage ws)
      (fn [msg]
        (when-let [data (.-data msg)]
        (put! out data))))
    (go
      (<! ready) ; Block until connection open
      {:in in :out out})))

