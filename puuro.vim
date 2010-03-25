" Vim syntax file
" Language:	puuro
" Maintainer:	Hannu Saransaari <hsaransa@cc.hut.fi>
" Last Change:	2009 Mar

syn clear

syn keyword xFunctions print new_continuation while if then else repeat iter
syn keyword xFunctions new_type active_frame require

syn keyword xPredefined builtin

syn region xLineComment start="#" keepend end="$"
syn region xRegionComment start="#-" end="-#" contains=xRegionComment

syn region xString start=/"/ skip=/\\"/ end=/"/

syn cluster xParenGroup contains=xParenError

syn region xCode transparent start="{" end="}" contains=ALL
syn region xList transparent start="\[" end="\]" contains=ALL
syn region xParen transparent start="(" end=")" contains=ALL

syn match xSpecialVariable "\$"
syn match xSpecialVariable "\$[cenr]"

syn match xSpecialVariable "\\[ftn]"

syn match xNumber "\d\d*"
syn match xNumber "\d*\.\d\d*"

syn match xSymbol "\'\h\w*"

syn match xVariable "\h\w*"

syn match xOperator "::"
syn match xOperator "\."
syn match xOperator "??"

highlight link xFunctions	Statement
highlight link xLineComment	Comment
highlight link xRegionComment	Comment
highlight link xString		String
highlight link xCode		Character
highlight link xList		Structure
highlight link xSpecialVariable	Special
highlight link xParenError	Error
highlight link xNumber		Number
highlight link xOperator	Operator
highlight link xSymbol		Identifier
highlight link xPredefined	Special

let b:current_syntax = "puuro"
