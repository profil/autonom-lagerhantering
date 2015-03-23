(ns agv-server.state)

(def state (agent {}))
(def users (agent {}))

(defn add-client
  [client stream]
  (if-not (contains? @state client)
    (send state assoc client {:stream stream})))

(defn remove-client
  [client]
  (if (contains? @state client)
    (send state dissoc client)))

(defn get-client
  [client]
  (get @state client))

(defn get-any-client
  []
  (let [s @state
        readys (filter #(:ready (second %)) s)
        r (into {} readys)]
    (first r)))

(defn set-client-ready
  [client]
  (send state assoc-in [client :ready] true))


(defn add-user
  [user stream]
  (send users assoc user {:stream stream}))

(defn remove-user
  [user]
  (send users dissoc user))

