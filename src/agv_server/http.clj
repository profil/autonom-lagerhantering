(ns agv-server.http
  (:require [aleph.http :as http]
            [manifold.stream :as s]
            [compojure.core :refer [defroutes GET]]
            [compojure.route :refer [resources not-found]]))

(defn wrap-index
  [handler]
  (fn  [request]
    (handler 
      (if  (= (:uri request)  "/")
        (assoc-in request  [:uri]  "/index.html")
        request))))

(def users (atom {}))

(defn websocket-handler
  [req]
  (let [s @(http/websocket-connection req)
        wkey (get-in req [:headers "sec-websocket-key"])]
    (s/on-closed s #(swap! users dissoc wkey))
    (s/put! s "hejsan")
    (swap! users assoc wkey s)
    (prn req)))

(defn start-server
  [handler port]
  (http/start-server handler {:port port}))

(defroutes handler
  (GET "/ws" [] websocket-handler)
  (resources "/")
  (not-found "Page not found"))

(def app
  (-> handler
      (wrap-index)))

