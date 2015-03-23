(ns agv-server.cljs.core
  (:require [cljs.core.async :refer [<! >! put! close! chan timeout]]
            [goog.dom :as dom]
            [goog.events :as events])
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
        (js/console.log "opening socket")
        (go-loop []
          (let [data (<! in)]
            (if (nil? data)
              (do (close! out)
                  (.close ws))
              (do (.send ws data)
                  (recur)))))))
    (set! (.-onclose ws)
      (fn []
        (js/console.log "closing socket")
        (close! in)
        (close! out)))
    (set! (.-onmessage ws)
      (fn [msg]
        (when-let [data (.-data msg)]
        (put! out data))))
    (go
      (<! ready) ; Block until connection open
      {:in in :out out})))


(go
  (let [{:keys [in out]} (<! (connect! "ws://localhost:3000/ws"))
        log (dom/getElement "log")
        stop (dom/getElement "stop")
        north (dom/getElement "north")
        south (dom/getElement "south")
        east (dom/getElement "east")
        west (dom/getElement "west")]

    (events/listen north "click"
      (fn [e] (put! in "north")))
    (events/listen south "click"
      (fn [e] (put! in "south")))
    (events/listen west "click"
      (fn [e] (put! in "west")))
    (events/listen east "click"
      (fn [e] (put! in "east")))
    (events/listen stop "click"
      (fn [e] (put! in "stop")))

    (loop [] ;; GET DATA
      (let [data (<! out)]
        (if-not (nil? data)
          (do
            (dom/append log (str data "\n"))
            (recur)))))))
    ;(go-loop [] ;; SEND DATA
     ; (<! (timeout 5000))
     ; (>! in "hejsan")
     ; (recur)))
      ;))
