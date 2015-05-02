(ns agv-server.cljs.core
  (:require [cljs.core.async :refer [<! >! put! chan timeout]]
            [reagent.core :as r :refer [atom]]
            [agv-server.cljs.ui :as ui]
            [agv-server.cljs.websocket :as ws]
            [cljs.reader :as reader])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(def state (r/atom {:connected? false}))
(def event-bus (chan))

; Start!
(ui/render state event-bus)


(go
  (let [{:keys [in out]} (<! (ws/connect! (str "ws://" (.-host js/location) "/ws")))]
    (swap! state assoc :connected? true)

    (go-loop []
      (let [[event msg] (<! event-bus)]
        (case event
          :search (>! in [:search msg])
          :accept (>! in [:accept msg])
          :abort (>! in [:abort msg])
          nil)
        (recur)))

    (go-loop [] ;; GET DATA
      (if-let [data (<! out)]
        (let [[event msg] (reader/read-string data)]
          (case event
            :agvs (swap! state assoc :agvs msg)
            :orders (swap! state assoc :orders msg)
            :warehouse (swap! state assoc :warehouse msg)
            :error (js/alert "Ogiltigt artikelnummer")
            nil)
          (recur))
        (swap! state assoc :connected? false)))))

