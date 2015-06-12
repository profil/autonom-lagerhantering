(ns agv-server.handlers
  (:require [agv-server.state :as st]
            [aleph.http :as http]
            [manifold.stream :as s]
            [clojure.edn :as edn]
            [agv-server.pathfinding :as p]
            [manifold.deferred :as d]))


(defn put-all-users
  [msg]
  (let [users (st/get-users)]
    (doseq [user (vals users)]
      (s/put! (:stream user) (pr-str msg)))))


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
    (s/put! s (pr-str [:agvs (keys (st/get-clients))]))
    (s/put! s (pr-str [:warehouse (st/get-map)]))
    (s/put! s (pr-str [:orders (st/get-orders)]))
    (add-watch st/orders :key
               (fn [k r os ns]
                 (s/put! s (pr-str [:orders ns]))))
    (s/connect-via s
      (fn [data]
        (let [[event msg] (edn/read-string data)]
          (case event
            :search (s/put! s (st/get-part msg))
            :accept (s/put! s (st/accept-order msg))
            :abort (s/put! s (st/abort-order msg))
            (d/future true))))
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
    (st/remove-client client)
    (put-all-users [:error (str "AGV " client " disconnected")])
    (put-all-users [:agvs (keys (st/get-clients))])
    (put-all-users [:warehouse (st/get-map)])))

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
      (do (put-all-users [:error "AGV ur kurs, manuell justering nödvändig."])
          (str "ERROR No coordinates given"))
      (let [ret (st/set-client-ready (:client @session)
                                     [(edn/read-string y)
                                      (edn/read-string x)])]
        (put-all-users [:agvs (keys (st/get-clients))])
        (put-all-users [:warehouse (st/get-map)])
        ret))))

(defn ok-handler
  [session]
  (with-session @session
    (st/lift-or-lower-ok (:client @session))))

(defn block-handler
  [[direction & params] session]
  (with-session @session
    (put-all-users [:warehouse (st/blocked-map (:client @session) direction)])
    (put-all-users [:error "Varning, hinder upptäckt."])))

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
      "BLOCK" (block-handler params session)
      (err-handler command params session))))
