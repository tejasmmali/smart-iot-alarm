let alarm = { hour:"07", minute:"30", ampm:"AM" };

export default function handler(req,res){

  if(req.method==="POST"){
    alarm=req.body;
    return res.status(200).json({ok:true});
  }

  res.status(200).json(alarm);
}