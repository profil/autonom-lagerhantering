(ns agv-server.state)

(def state (atom {}))

(defn add-client
  [client]
  (if-not (contains? @state client)
    (swap! state assoc client {})))

(defn remove-client
  [client]
  (if (contains? @state client)
    (swap! state dissoc client)))

(defn get-client
  [client]
  (get @state client))
