#curl -d '4a6c164b5f460e69ae93f96e0f7a6f6376057a40 : 0 : 2265027 : 45 : ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby ruby $`I' 127.0.0.1:4567/stats

require 'rubygems'
require 'sinatra'
require 'dm-core'
require 'dm-timestamps'

DataMapper.setup(:default, "sqlite3://#{Dir.pwd}/entries.sqlite3")

class Entry
  include DataMapper::Resource

  property :id,          Integer, :serial => true    # primary serial key
  property :ip,          String
  property :hash,        String
  property :case_perm,   String
  property :suffix_perm, String
  property :distance,    Integer
  property :phrase,      Text
  property :sfx,         String

  property :created_at, DateTime
  property :updated_at, DateTime

end

DataMapper.auto_migrate!

get '/stats' do
  #display results
end

post '/stats' do
  params.to_s =~ /(.{40}) : (\d+) : (\d+) : (\d+) : (.+)(.{5})/
  
  entry = { :ip               => request.ip,
            :hash             => $1, 
            :case_perm        => $2, 
            :suffix_perm      => $3, 
            :distance         => $4, 
            :phrase           => $5,
            :sfx              => $6}

  #insert entry
  Entry.create(entry)
end

