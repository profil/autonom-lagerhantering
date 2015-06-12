(ns agv-server.cljs.ui
  (:require [cljs.core.async :refer [<! >! put! close! chan timeout]]
            [reagent.core :as r :refer [atom]])
  (:require-macros [cljs.core.async.macros :refer [go go-loop]]))

(defn input-field
  [app-db event-bus]
  (let [val (atom "")]
    (fn []
      [:input {:type "text" :value @val
               :placeholder "Artikelnr.."
               :autoFocus "autoFocus"
               :disabled (not (get @app-db :connected?))
               :on-change #(reset! val (-> % .-target .-value))
               :on-key-down #(case (.-which %)
                               13 (do (put! event-bus [:search @val])
                                      (reset! val ""))
                               27 (reset! val "")
                               nil)}])))

(defn agv-list
  [app-db event-bus]
  [:div#list.sidebar-content
   [:h3 "Tillgängliga AGV"]
   [:ul
    (for [agv (get @app-db :agvs)]
      ^{:key agv} [:li agv])]])

(defn main-content
  [app-db event-bus]
  [:div#content
   (for [[id {:keys [agv done shelf]}] (get @app-db :orders)]
     ^{:key id}
     [:div.order {:class (if done "accept")}
      [:h4 "#" id]
      (if done
        [:button.accept {:on-click #(put! event-bus [:accept id])} "Kvittera"]
        [:button.abort {:on-click #(put! event-bus [:abort id])} "Avbryt"])
      [:p (if (nil? agv) "Väntar på AGV" (str "Hämtas av: " agv))]])])

(defn app
  [app-db event-bus]
  [:div  {:class (if-not (get @app-db :connected?) "disabled")}
   [:header [:h1 "Sök och hämta produkt ur varulager"]]
   [:main
    [:p#search [input-field app-db event-bus]]
    [:section
     [:div.sidebar
      [agv-list app-db event-bus]
      [:div#warehouse.sidebar-content
       [:h3 "Realtidskarta"]
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
                       :fill (case (-> cell name (.split "-") first)
                               "none"    "#fff"
                               "free"    "#eee"
                               "agv"     "#286090"
                               "s"       "#31b0d5"
                               "station" "#0f0"
                               "block"   "#d9534f")}])])])]]
     [main-content app-db event-bus]]]])

(defn render
  [app-db event-bus]
  (r/render [app app-db event-bus] (.-body js/document)))
