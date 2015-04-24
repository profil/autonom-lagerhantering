(ns agv-server.pathfinding
  (:require [clojure.data.priority-map :as pm]))


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

(defn directions
  [path]
  (let [dirs
        {[ 1  0] "SOUTH"
         [-1  0] "NORTH"
         [ 0  1] "EAST"
         [ 0 -1] "WEST"}]
    (loop [[f s & xs] path
           ret []]
      (if-not (or (nil? f) (nil? s))
        (recur (cons s xs) (conj ret (dirs [(- (s 0) (f 0)) (- (s 1) (f 1))])))
        ret))))

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



