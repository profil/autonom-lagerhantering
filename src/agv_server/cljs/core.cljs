(ns agv-server.cljs.core
  (:require [cljs.core.async :refer [<! >! put! chan timeout]]
            [reagent.core :as r :refer [atom]]
            [agv-server.cljs.ui :as ui]
            [agv-server.cljs.websocket :as ws]
            [cljs.reader :as reader])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(def state (r/atom {:agvs #{}
                    :connected? false
                    :warehouse [[ [:free] [:free] [:free] [:free] ]
                                [ [:free] [:free] [:free] [:free] ]
                                [ [:free] [:free] [:free] [:free] ]
                                [ [:free] [:free] [:free] [:free] ]]}))

(def event-bus (chan))

(defn update-map
  [m {:keys [to from id]}]
  (let [m1 (assoc-in m to 4)]
    (if-not (nil? from)
      (assoc-in m1 from 1)
      m1)))

; Start!
(ui/render state event-bus)

(go
  (let [{:keys [in out]} (<! (ws/connect! "ws://localhost:3000/ws"))]
    (swap! state assoc :connected? true)

    (loop [] ;; GET DATA
      (let [data (<! out)]
        (if-not (nil? data)
          (let [[event msg] (reader/read-string data)]
            (case event
              :connect (swap! state update-in [:agvs] conj msg)
              :disconnect (swap! state update-in [:agvs] disj msg)
              :move (swap! state update-in [:warehouse] #(update-map % msg)))
            (recur))
          (swap! state assoc :connected? false))))))

