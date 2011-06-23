// __BEGIN_LICENSE__
// Copyright (C) 2006-2010 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__


#ifndef __VW_PLATE_DETAIL_BLOBSTORE_H__
#define __VW_PLATE_DETAIL_BLOBSTORE_H__

#include <vw/Plate/Blob.h>
#include <vw/Plate/Datastore.h>
#include <vw/Core/Log.h>
#include <vw/Core/Cache.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>


namespace vw { namespace platefile {
  class Blob;
  class ReadBlob;
namespace detail {
  class Index;
  class BlobOpener;

class Blobstore : public Datastore {
  private:
    boost::shared_ptr<Index> m_index;

    //typedef boost::multi_index_container<boost::reference_wrapper<ReadBlob>,
    typedef boost::multi_index_container<boost::shared_ptr<ReadBlob>,
              boost::multi_index::indexed_by<boost::multi_index::sequenced<>, 
                                             boost::multi_index::hashed_unique<boost::multi_index::const_mem_fun<ReadBlob, const std::string&, &ReadBlob::filename> > > > read_cache_t;

    typedef read_cache_t::nth_index<0>::type read_cache_by_age_t;
    typedef read_cache_t::nth_index<1>::type read_cache_by_filename_t;

    typedef std::map<uint32, boost::shared_ptr<Blob> > write_cache_t;

    read_cache_t  m_read_cache;
    write_cache_t m_write_cache;
    vw::Mutex m_mutex;

    boost::shared_ptr<ReadBlob>  open_read_blob(uint32 blob_id);
    boost::shared_ptr<Blob>     open_write_blob(uint32 blob_id);

    void init();
  public:
    Blobstore(const Url& u);
    Blobstore(const Url& u, const IndexHeader& d);

    virtual Transaction transaction_begin(const std::string& description, TransactionOrNeg transaction_id_override = -1);
    virtual void transaction_end(Transaction transaction_id, bool update_read_cursor);

    virtual TileSearch&     head(TileSearch& buf, uint32 level, uint32 row, uint32 col, TransactionRange range, uint32 limit = 0);
    virtual TileSearch&     head(TileSearch& buf, uint32 level,   const BBox2u& region, TransactionRange range, uint32 limit = 0);
    virtual TileSearch& populate(TileSearch& hdrs);

    //virtual Url map_to_url(uint32 level, uint32 row, uint32 col, Transaction id, std::string filetype);
    //virtual Url map_to_url(const TileHeader& t);

    virtual WriteState* write_request(const Transaction& id);
    virtual void write_update(WriteState& state, uint32 level, uint32 row, uint32 col, const std::string& filetype, const uint8* data, uint64 size);
    virtual void write_complete(WriteState& id);
    virtual void flush();

    virtual IndexHeader index_header() const;

    // LOGGING
    virtual Datastore::Logger audit_log() const;
    virtual Datastore::Logger error_log() const;
};


}}} // vw::platefile::detail

#endif