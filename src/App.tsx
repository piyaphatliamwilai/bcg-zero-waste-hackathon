import { useState } from 'react'
import './App.css'

function App() {

  const [waste, setWaste] = useState("0")
  const [carbonFootprint, setCarbonFootprint] = useState("0")
  const [money, setMoney] = useState("0")

  setInterval(() => {
    parseVal()
  }, 5000)

  async function parseVal() {
    const values: Response = await fetch('https://script.google.com/macros/s/AKfycbxfvJcKzVFr_DViMr-fgs2UY-JRQTrs2LHlvcu4GHvmlaebkTJoccUN2AGE3h7uDGNB/exec')
    const parsedText: string = await values.text()
    setWaste(parsedText.split(",")[0])
    setCarbonFootprint(parsedText.split(",")[1])
    setMoney(parsedText.split(",")[2])
  }

  return (
    <>
        <div className='flex-row  bg-stone-950 min-h-screen p-4 space-y-4' id='background'>
          <div className="flex-row rounded-xl backdrop-blur-sm bg-black bg-opacity-40 h-max min-w-max p-4">
            <h1 className='text-2xl'>แค่ไม่ RUN มันก็ไม่ ERROR</h1>
            <h1 className='text-l'>Arduino IOT Hackathon 2024 @ KMITL</h1>
          </div>
          <div className="flex rounded-xl backdrop-blur-sm space-x-4 bg-black bg-opacity-40 min-h-[60vh] min-w-max p-4">
            <div className='rounded-l-3xl bg-white py-32 space-y-16 bg-opacity-10 w-[33%] min-h-max place-items-center'>
              <h1 className='text-9xl font-bold text-center'>{waste}</h1>
              <h1 className='text-4xl text-center'>จำนวนขยะที่รีไซเคิล</h1>
            </div>
            <div className='bg-white py-32 space-y-16 bg-opacity-10 w-[33%] min-h-max place-items-center'>
              <h1 className='text-9xl font-bold text-center'>{money}</h1>
              <h1 className='text-4xl text-center'>ราคาที่เราซื้อขยะทั้งหมด</h1>
            </div>
            <div className='bg-white py-32 space-y-16 bg-opacity-10 w-[33%] min-h-max place-items-center'>
              <h1 className='text-9xl font-bold text-center'>{carbonFootprint}</h1>
              <h1 className='text-4xl text-center'>Carbon Footprint ที่รักษาได้</h1>
            </div>
          </div>
          <h1 className='text-l text-center'>ข้อมูลมีการ Update ทุก 5 วินาที</h1>
          <div className="flex-row rounded-xl backdrop-blur-sm space-y-8  bg-black bg-opacity-40 min-w-max p-8">
            <h1 className='text-6xl'>ขาดเงินทอนเท่าไหร่?</h1>
            <h1 className='text-3xl'>เช็คจำนวนเงินที่คุณขาด เพียงแค่ใส่เบอร์คุณตรงนี้</h1>
            <input type="string" className="outline-1 bg-opacity-60 bg-black rounded-sm outline outline-zinc-700 px-2 text-lg" />
          </div>
        </div>
    </>
  )
}

export default App
