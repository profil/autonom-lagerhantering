(ns agv-server.cljs.core
  (:require [cljs.core.async :refer [<! >! put! chan timeout]]
            [reagent.core :as r :refer [atom]]
            [agv-server.cljs.ui :as ui]
            [agv-server.cljs.websocket :as ws]
            [cljs.reader :as reader])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(def state (r/atom {:agvs #{}
                    :connected? false
                    :warehouse [[ [:free] [:free] [:free] [:free] [:free] [:free] [:free] [:free]]
                                [ [:free] [:none] [:none] [:none] [:none] [:none] [:none] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:none] [:none] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:free] [:free] [:free]]
                                [ [:free] [:none] [:free] [:free] [:free] [:free] [:free] [:free]]
                                [ [:free] [:free] [:free] [:free] [:free] [:free] [:free] [:free]]
                                [ [:free] [:free] [:free] [:free] [:free] [:free] [:free] [:free]]]}))

(defn update-map
  [m path]
  (let [m1 ((fn [map [ks v & kvs]]
             (let [ret (assoc-in map ks v)]
              (if kvs
                (recur ret kvs)
                ret)))
            m (vec (interleave path (repeat [:path]))))]
    m1))
  ;[m {:keys [to from id]}]
  ;(let [m1 (assoc-in m to 4)]
  ;  (if-not (nil? from)
  ;    (assoc-in m1 from 1)
  ;    m1)))

(def event-bus (chan))

; Start!
(ui/render state event-bus)

(go
  (let [{:keys [in out]} (<! (ws/connect! "ws://localhost:3000/ws"))]
    (swap! state assoc :connected? true)

    (go-loop []
      (let [[event msg] (<! event-bus)]
        (case event
          :search (>! in [:go-to {:from [0 0] :to [(-> msg first reader/read-string)
                                                   (-> msg nnext first reader/read-string)]}])
          nil)
        (recur)))

    (go-loop [] ;; GET DATA
      (let [data (<! out)]
        (if-not (nil? data)
          (let [[event msg] (reader/read-string data)]
            (case event
              :connect (swap! state update-in [:agvs] conj msg)
              :disconnect (swap! state update-in [:agvs] disj msg)
              :move (swap! state update-in [:warehouse] #(update-map % msg))
              :path (swap! state update-in [:warehouse] #(update-map % msg)))
            (recur))
          (swap! state assoc :connected? false))))))

