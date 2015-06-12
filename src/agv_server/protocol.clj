(ns agv-server.protocol
  (:require [aleph.tcp :as tcp]
            [manifold.stream :as s]
            [manifold.deferred :as d]
            [gloss.core :as g :refer [defcodec]]
            [gloss.io :as io]
            [agv-server.handlers :refer [protocol-handler disconnect-handler]]
            [agv-server.state :as st]))

(defcodec protocol (g/string :utf-8 :delimiters ["\r\n"]))

(defn wrap-duplex-stream
  [protocol s]
  (let [out (s/stream)]
    (s/connect (s/map #(io/encode protocol %) out) s)
    (s/splice out (io/decode-stream s protocol))))

(defn start-server
 [handler port]
  (tcp/start-server
    (fn [s info]
      (handler (wrap-duplex-stream protocol s) info))
    {:port port}))

(defn handler
  [s info]
  (let [session (atom {:stream s})]
    ;; Remove client when disconnected
    (s/on-closed s #(disconnect-handler session))

    ;; Here is where the work is done
    (s/connect-via s
      (fn [msg]
        (if-let [result (protocol-handler msg session)]
          (s/put! s result)
          (d/future true)))
      s)

    ;; Ping/Pong message handler
    (swap! session assoc :pong true)
    (s/connect (s/periodically 25000 25000
      #(when-not (s/closed? s)
          (if (:pong @session)
            (do
              (swap! session assoc :pong false)
              (str "PING"))
            (do
              (s/close! s)
              (println "Ping timeout" @session))))) s)))

