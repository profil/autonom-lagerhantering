(ns agv-server.pathfinding
  (:require [clojure.data.priority-map :as pm]))


(def warehouse (atom [[[:free] [:free] [:free] [:free]]
                      [[:shelf] [:shelf] [:shelf] [:free]]
                      [[:free] [:free] [:free] [:free]]
                      [[:free] [:free] [:free] [:free]]]))

(defn manhattan
  [[x1 y1] [x2 y2]]
  (+ (Math/abs (- x2 x1)) (Math/abs (- y2 y1))))

(defn cost
  [g current end]
  (let [h (manhattan current end)
        f (+ g h)]
    [f g]))


(defn neighbours
  [map [y x] [ey ex]]
  (reduce (fn [res [dy dx]]
            (if (and (= dy ey) (= dx ex))
              (reduced [[ey ex]])
              (if (and (>= dx 0)
                       (>= dy 0)
                       (< dx (count (first map)))
                       (< dy (count map))
                       (= :free (get-in map [dy dx 0])))
                (conj res [dy dx])
                res)))
          []
          [[y (- x 1)] [(+ y 1) x] [y (+ x 1)] [(- y 1) x]]))


(defn rebuild-path
  [current visited]
  (reverse
    (loop [path [current]
           node (visited current)]
      (if (nil? node)
        path
        (recur (conj path node) (visited node))))))

(defn astar
  [map start end]
  (let [width (count (first map))
        height (count map)]
    (loop [frontier (pm/priority-map-keyfn first start (cost 0 start end))
           visited {}]
      (if-let [[current [f g parent]] (peek frontier)]
        (let [visited (assoc visited current parent)]
          (if-not (= current end)
            (let [neighbours (neighbours map current end)
                  frontier (reduce
                             (fn [frontier neighbour]
                               (if-not (contains? visited neighbour)
                                 (let [[nf ng] (cost (+ g 1) neighbour end)]
                                   (if (or (not (contains? frontier neighbour))
                                           (-> neighbour
                                               frontier 
                                               second
                                               (< ng)))
                                     (assoc frontier neighbour
                                            [nf ng current])
                                     frontier))
                                 frontier))
                             (pop frontier) neighbours)]
              (recur frontier visited))
            (rebuild-path current visited)))))))



