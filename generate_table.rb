def hamming_distance(a, b)
  c = (a ^ b)
  total = 0
  while c > 0
    total += ((c % 2)==0) ? 0 : 1
    c >>= 1
  end  
  return total
end

puts '#include "table_generated.h"'
puts "void build_hamming_table(){"
puts "extern int HAM_TABLE[256][256];"

(0..255).each do |i|
  (0..255).each do |j|
    puts "HAM_TABLE[#{i}][#{j}] = #{hamming_distance(i,j)};"
  end
end

puts "}"
