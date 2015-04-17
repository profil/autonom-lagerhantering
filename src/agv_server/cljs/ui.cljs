(ns agv-server.cljs.ui
  (:require [cljs.core.async :refer [<! >! put! close! chan timeout]]
            [reagent.core :as r :refer [atom]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(defn app
  [app-db event-bus]
  [:div
   [:h1 "Autonom lagerhantering"]
   [:p "Connection to server: " (str (get @app-db :connected?))]
   (let [warehouse (get @app-db :warehouse)
         size 30]
     [:svg {:width (* size (count (first warehouse)))
            :height (* size (count warehouse))}
      (for [[row-index row] (map-indexed vector warehouse)]
        ^{:key row-index}
        [:g {:transform (str "translate(0, " (* row-index size) ")")}
         (for [[col-index cell] (map-indexed vector row)]
           ^{:key (+ col-index row-index)}
           [:rect {:x (* col-index size)
                   :width (- size (/ size 10))
                   :height (- size (/ size 10))
                   :fill (case (first cell)
                           :none  "#fff"
                           :free  "#eee"
                           :agv   "#286090"
                           :shelf "#31b0d5"
                           4 "#5cb85c")}])])])
   [:ul
    (for [agv (get @app-db :agvs)]
      ^{:key agv} [:li agv])]])

(defn render
  [app-db event-bus]
  (r/render [app app-db event-bus] (.-body js/document)))
