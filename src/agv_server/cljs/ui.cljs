(ns agv-server.cljs.ui
  (:require [cljs.core.async :refer [<! >! put! close! chan timeout]]
            [reagent.core :as r :refer [atom]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(defn input-field
  [app-db event-bus]
  (let [val (atom "")]
    (fn []
      [:input {:type "text" :value @val
               :disabled (not (get @app-db :connected?))
               :on-change #(reset! val (-> % .-target .-value))
               :on-key-down #(case (.-which %)
                               13 (put! event-bus [:search @val])
                               27 (reset! val "")
                               nil)}])))

(defn app
  [app-db event-bus]
  [:div {:class (if-not (get @app-db :connected?) "disabled")}
   [:h1 "Autonom lagerhantering"]
   [:p#search [input-field app-db event-bus]]
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
                           :path  "#c1ddf9")}])])])
   [:ul
    (for [agv (get @app-db :agvs)]
      ^{:key agv} [:li agv])]])

(defn render
  [app-db event-bus]
  (r/render [app app-db event-bus] (.-body js/document)))
