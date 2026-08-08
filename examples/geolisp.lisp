(fun v3 (x y z) 
    (list x y z)
)

(fun trig (a b c)
    (list a b c)
)

(fun x_val (v)
    (car v)
)

(fun y_val (v)
    (print (car (cdr v)))
)

(fun z_val (v)
    (car (cdr (cdr v)))
)

(fun plane (loc size)
    "2D plane on x and y axis"
    (let ((m (createmesh)))
        (push_vert m loc)
        (trace "debug")
        (push_vert m (v3 
            (x_val loc) 
            (add (y_val loc) (y_val size)) 
            (z_val loc)
        ))
        (trace "error")
        (push_vert m (v3 
            (add (x_val loc) (x_val size)) 
            (add (y_val loc) (y_val size)) 
            (z_val loc)
        ))
        (push_vert m (v3 
            (add (x_val loc) (x_val size)) 
            (y_val loc) 
            (z_val loc)
        ))

        (push_trig m (trig 2 1 0))
        (push_trig m (trig 0 2 3))
    )
)

; (add (z_val (v3 1 3 -20)) (x_val (v3 1 1 2)))
(plane (v3 1 3 -20) (v3 32 32 1))

; (add (car (list 0 0 -20)) (car (list 1 1 2)))
; (y_val (v3 1 2 3))
; (v3 1 2 3)

; ((1  . (2  . (3  . ))) . )
; (list 1 2 3)
; ((1  . (2  . (3  . ))) . )

; (octree (createmesh) (v3 0 0 -20) 5)