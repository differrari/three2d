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

(fun cube (loc size)
    "2D plane on x and y axis"
    (let ((m (createmesh)))
        (push_vert m loc)
        (push_vert m (v3 
            (x_val loc) 
            (add (y_val loc) (y_val size)) 
            (z_val loc)
        ))
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

        (push_vert m (v3 
            (x_val loc)
            (y_val loc) 
            (add (z_val loc) (z_val size)) 
        ))
        (push_vert m (v3 
            (x_val loc) 
            (add (y_val loc) (y_val size)) 
            (add (z_val loc) (z_val size))
        ))
        (push_vert m (v3 
            (add (x_val loc) (x_val size)) 
            (add (y_val loc) (y_val size)) 
            (add (z_val loc) (z_val size))
        ))
        (push_vert m (v3 
            (add (x_val loc) (x_val size)) 
            (y_val loc) 
            (add (z_val loc) (z_val size))
        ))

        (push_trig m (trig 2 1 0))
        (push_trig m (trig 3 2 0))

        (push_trig m (trig 1 5 6))
        (push_trig m (trig 1 6 2))

        (push_trig m (trig 0 1 4))
        (push_trig m (trig 1 4 5))

        (push_trig m (trig 3 2 6))
        (push_trig m (trig 3 6 7))

        (push_trig m (trig 0 4 7))
        (push_trig m (trig 0 3 7))

        (push_trig m (trig 4 5 6))
        (push_trig m (trig 4 6 7))
    )
)

; (add (z_val (v3 1 3 -20)) (x_val (v3 1 1 2)))
(cube (v3 1 3 -2) (v3 32 33 1))

; (add (car (list 0 0 -20)) (car (list 1 1 2)))
; (y_val (v3 1 2 3))
; (v3 1 2 3)

; ((1  . (2  . (3  . ))) . )
; (list 1 2 3)
; ((1  . (2  . (3  . ))) . )

; (octree (createmesh) (v3 0 0 -20) 5)