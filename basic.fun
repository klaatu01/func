function Main () 
variables inp,res
begin
  read inp;
  res := 0;
  while Less(0,inp) loop
     res := Plus(res,inp);
     inp := Minus(inp,1);
  end loop; 
  write res;
end function;
