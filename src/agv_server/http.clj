(ns agv-server.http
  (:require [aleph.http :as http]
            [manifold.stream :as s]
            [agv-server.state :as st]
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

(defn handle-direction
  [dir]
  (if-let [[client {stream :stream}] (st/get-any-client)]
    (do (s/put! stream dir)
      (str "Sending " dir " to " client))
    (str "No available AGV!")))

(defn websocket-handler
  [req]
  (let [s @(http/websocket-connection req)
        wkey (get-in req [:headers "sec-websocket-key"])]
    (s/put! s "HELLO!!")
    (swap! users assoc wkey s)
    (s/on-closed s #(swap! users dissoc wkey))
    (s/connect-via s
      (fn [msg]
        (case msg
          "stop" (s/put! s (handle-direction "STOP"))
          "north" (s/put! s (handle-direction "NORTH"))
          "west" (s/put! s (handle-direction "WEST"))
          "east" (s/put! s (handle-direction "EAST"))
          "south" (s/put! s (handle-direction "SOUTH"))
          (s/put! s (str "got unknown" msg))))
      s)))

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

