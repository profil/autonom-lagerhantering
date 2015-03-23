(ns agv-server.handlers
  (:require [agv-server.state :as st]
            [aleph.http :as http]
            [manifold.stream :as s]))

;; WebSocket!
(def users (agent {}))

(defn handle-direction
  [dir]
  (if-let [[client {stream :stream}] (st/get-any-client)]
    (do (s/put! stream dir)
      (str "Sending " dir " to " client))
    (str "No available AGV!")))

(defn websocket-handler
  [req]
  (let [s @(http/websocket-connection req)
        user (get-in req [:headers "sec-websocket-key"])]
    (s/put! s "HELLO!!")
    (st/add-user user s)
    (s/on-closed s #(st/remove-user user))
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



;; TCP
(defmacro with-session
  [session & body]
  `(if-let [c# (:client ~session)]
     (do ~@body
         (str "OK"))
     (str "ERROR Not authenticated")))

(defn disconnect-handler
  [client]
  (st/remove-client client))

(defn login-handler
  [[client & params] session]
  (if (:client @session)
    (str "ERROR You are already authenticated")
    (if-let [status (st/add-client client (:stream @session))]
      (do (swap! session assoc :client client)
          (str "OK"))
      (str "ERROR Client already exist"))))

(defn pong-handler
  [session]
  (with-session @session
    (swap! session assoc :pong true)))

(defn ready-handler
  [[coords & params] session]
  (with-session @session
    (st/set-client-ready (:client @session))))

(defn ok-handler
  [session]
  (with-session @session nil))

(defn err-handler
  [command params session]
  (str "ERROR Unknown command, session: " @session))

(defn protocol-handler
  [msg session]
  (let [[command & params] (clojure.string/split msg #"\s+")]
    (case command
      "LOGIN" (login-handler params session)
      "PONG" (pong-handler session)
      "READY" (ready-handler params session)
      "OK" (ok-handler session)
      (err-handler command params session))))
