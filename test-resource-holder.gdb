break _alloc_new
commands
silent
print curr
cont
end

break resource_holder_push
commands
silent
print *holder
cont
end

run 10
quit
