(ns agv-server.handlers
  (:require [agv-server.state :as st]
            [aleph.http :as http]
            [manifold.stream :as s]
            [clojure.edn :as edn]))


(defn put-all-users
  [msg]
  (let [users (st/get-users)]
    (doseq [user (vals users)]
      (s/put! (:stream user) (str msg)))))


;; WebSocket!
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
    (st/add-user user s)
    (s/on-closed s #(st/remove-user user))
    (doseq [agv (st/get-clients)]
      (s/put! s (str [:connect (key agv)]))
      (s/put! s (str [:move {:from nil :to (:ready (val agv)) :id (key agv)}])))
    (s/connect-via s
      (fn [data]
        (let [[event msg] (edn/read-string data)]
          (case msg
            "stop" (s/put! s (handle-direction "STOP"))
            "north" (s/put! s (handle-direction "NORTH"))
            "west" (s/put! s (handle-direction "WEST"))
            "east" (s/put! s (handle-direction "EAST"))
            "south" (s/put! s (handle-direction "SOUTH"))
            (s/put! s (str "got unknown" msg)))))
      s)))



;; TCP
(defmacro with-session
  [session & body]
  `(if-let [c# (:client ~session)]
     (let [result# (do ~@body)]
       (if (string? result#)
         result#
         (str "OK")))
     (str "ERROR Not authenticated")))

(defn disconnect-handler
  [session]
  (when-let [client (:client @session)]
    (put-all-users [:disconnect client])
    (st/remove-client client)))

(defn login-handler
  [[client & params] session]
  (if (nil? client)
    (str "ERROR Missing name")
    (if (:client @session)
      (str "ERROR You are already authenticated")
      (if-let [status (st/add-client client (:stream @session))]
        (do (swap! session assoc :client client)
            (str "OK"))
        (str "ERROR Client already exist")))))

(defn pong-handler
  [session]
  (with-session @session
    (swap! session assoc :pong true)))

(defn ready-handler
  [[x y & params] session]
  (with-session @session
    (if (nil? y)
      (str "ERROR No coordinates given")
      (let [[from to] (st/set-client-ready
                                (:client @session)
                                [(edn/read-string y) (edn/read-string x)])]
          (put-all-users [:connect (:client @session)])
          (put-all-users [:move {:from from :to to :id (:client @session)}])))))

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
