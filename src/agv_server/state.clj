(ns agv-server.state)

(def state (atom {}))

(defn add-client
  [client stream]
  (if-not (contains? @state client)
    (swap! state assoc client {:stream stream})))

(defn remove-client
  [client]
  (if (contains? @state client)
    (swap! state dissoc client)))

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
  (swap! state assoc-in [client :ready] true))
