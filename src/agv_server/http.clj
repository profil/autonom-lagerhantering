(ns agv-server.http
  (:require [aleph.http :as http]
            [manifold.stream :as s]
            [agv-server.state :as st]
            [agv-server.handlers :refer [websocket-handler]]
            [compojure.core :refer [defroutes GET]]
            [compojure.route :refer [resources not-found]]))

(defn wrap-index
  [handler]
  (fn  [request]
    (handler 
      (if  (= (:uri request)  "/")
        (assoc-in request  [:uri]  "/index.html")
        request))))

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

