<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka clanku
 *   */
class ClankyRepository extends Repository
{
    public function addClanek($values) {
	$this->getTable()->insert(array(
	    'Nazev' => $values->nazev,
	    'Uvod' => $values->uvod,
	    'Text' => $values->text,
	    'Autor' => $values->autor,
	    'Datum' => date("Y-m-d")
	));
    }

    public function deleteClanek($id)
    {
		$this->findBy(array('ID_clanku'=>$id))->delete();
    }

}
