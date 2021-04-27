; AisleRiot - isabel.scm
; Copyright (C) 2001 Rosanna Yuen <zana@webwynk.net>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (aisleriot interface) (aisleriot api))

(define first-wall-slot-id 0)
(define last-wall-slot-id 47)
(define wall '(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
                 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33
                 34 35 36 37 38 39 40 41 42 43 44 45 46 47
                 ))
(define first-target-slot-id 0)

(define (new-game)
  (initialize-playing-area)
  ;(make-standard-deck)
  (make-standard-double-deck)
  (shuffle-deck)

; wall
  (add-empty-normal-slots 8)
  
  (add-carriage-return-slot)
  (set! VERTPOS (- VERTPOS 0.5))
  (set! HORIZPOS (+ HORIZPOS 0.5))
  (add-empty-normal-slots 8)
  
  (add-carriage-return-slot)
  (set! VERTPOS (- VERTPOS 0.5))
  (add-empty-normal-slots 8)
  
  (add-carriage-return-slot)
  (add-carriage-return-slot)
  (set! VERTPOS (- VERTPOS 0.5))
  (set! HORIZPOS (+ HORIZPOS 0.5))
  (add-empty-normal-slots 8)
  
  (add-carriage-return-slot)
  (set! VERTPOS (- VERTPOS 0.5))
  (add-empty-normal-slots 8)
  
  (add-carriage-return-slot)
  (set! VERTPOS (- VERTPOS 0.5))
  (set! HORIZPOS (+ HORIZPOS 0.5))
  (add-empty-normal-slots 8)
  
  
; deck
 ; (add-carriage-return-slot)
 ; (set! VERTPOS (+ VERTPOS 0.5))
 ; (add-blank-slot)
 ; (add-blank-slot)
 ; (add-normal-slot DECK)

;  (add-normal-slot DECK)

  ;(deal-cards-face-up 48  wall)  
  
 (deal-cards-face-up-from-deck DECK  '(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
                                	0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
					24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
					24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
                                 	0 1 2 3 4 5 6 7 ))
  ;(deal-cards-face-up-from-deck DECK '(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23))


  ;(list 8 8) )
  (list 8 6) )

(define (add-empty-normal-slots n)
  (if (> n 0)
      (begin (add-normal-slot '())
             (add-empty-normal-slots (- n 1)))))

(define (button-pressed slot-id card-list)
  (not (empty-slot? slot-id)))

(define (droppable? start-slot card-list end-slot)
  (and (not (empty-slot? end-slot))
       (not (= start-slot end-slot))
       (= (get-value (car card-list))
	  (get-value (get-top-card end-slot)))))

(define (button-released start-slot card-list end-slot)
  (and (droppable? start-slot card-list end-slot)
       (remove-card end-slot)
       (add-to-score! 2)
       ;(or (empty-slot? start-slot)
       ;   (flip-top-card start-slot))
       ;(or (empty-slot? end-slot)
	;   (flip-top-card end-slot))
	))

(define (button-clicked slot-id)
  #f)

(define (button-double-clicked slot-id)
  #f)

(define (game-continuable)
  (and (not (game-won))
       (get-hint)))

(define (game-won)
  (= (get-score) 104))

(define (check-for-pairs slot1 slot2)
  (cond ((= slot1 13)
	 #f)
	((or (= slot2 13)
	     (empty-slot? slot1))
	 (check-for-pairs (+ 1 slot1) (+ 2 slot1)))
	((and (not (empty-slot? slot2))
	      (= (get-value (get-top-card slot1) ) (get-value (get-top-card slot2)) ) )
		;(cards-eq? (get-top-card slot1) (get-top-card slot2) ))
	 (hint-move slot1 1 slot2))
	(#t (check-for-pairs slot1 (+ 1 slot2)))))

(define (get-hint)
  (check-for-pairs 0 1))

(define (get-options) 
  #f)

(define (apply-options options) 
  #f)

(define (timeout) 
  #f)

(set-features droppable-feature)

(set-lambda new-game button-pressed button-released button-clicked
button-double-clicked game-continuable game-won get-hint get-options
apply-options timeout droppable?)
